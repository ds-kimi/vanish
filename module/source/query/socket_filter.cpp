// Derived from gmsv_spoof by Daniel Almeida (BSD 3-Clause), see
// licenses/gmsv_spoof.txt. Reduced to the query path: the original also
// carried a threaded socket, packet sampling, a firewall and packet
// validation, none of which this addon needs.

#include "socket_filter.hpp"
#include "answer.hpp"
#include "challenge.hpp"
#include "game_socket.hpp"
#include "packet.hpp"
#include "recv_hook.hpp"
#include "replies.hpp"
#include "state.hpp"

#include "../platform/memory.hpp"

#include <tier0/dbg.h>

#if !defined SYSTEM_WINDOWS
#include <errno.h>
#endif

namespace Vanish
{
	namespace Query
	{
		namespace SocketFilter
		{
			// queries answered per engine read before the rest of the queue is
			// left for the next one, so a flood cannot hold the frame forever
			const int MAX_ANSWERED_PER_READ = 32;

			static RecvHook::Function original = nullptr;

			/// Tells the engine the socket had nothing to read, which is how a
			/// packet handled here disappears from its point of view.
			static RecvHook::Result Swallow( )
			{
#if defined SYSTEM_WINDOWS
				WSASetLastError( WSAEWOULDBLOCK );
#else
				errno = EWOULDBLOCK;
#endif
				return -1;
			}

			static RecvHook::Result VANISH_RECVFROM_CALL Detour( RecvHook::Socket socket, char *buffer,
				RecvHook::Length length, int flags, sockaddr *from, RecvHook::AddressLength *fromlen )
			{
				if( !Enabled( ) || from == nullptr || fromlen == nullptr )
					return original( socket, buffer, length, flags, from, fromlen );

				// an answered query is followed by the next datagram rather than
				// by "would block", which the engine takes as an empty socket and
				// stops reading for the frame
				for( int answered = 0; answered < MAX_ANSWERED_PER_READ; ++answered )
				{
					const RecvHook::Result received = original( socket, buffer, length, flags, from, fromlen );
					const size_t size = received > 0 ? static_cast<size_t>( received ) : 0;
					const char type = Packet::QueryType( buffer, size );
					if( type == 0 || Packet::FromLoopback( from ) || !GameSocket::Is( socket ) )
						return received;

					Answer::Query( socket, from, *fromlen, type, buffer, size );
				}

				return Swallow( );
			}

			bool Attach( )
			{
				if( original != nullptr )
					return true;

				if( !Replies::Initialize( ) )
				{
					Warning( "[vanish] missing engine interfaces, query hiding is disabled\n" );
					return false;
				}

				Challenge::Initialize( );
				Replies::Refresh( );

				original = RecvHook::Install( Detour );
				if( original == nullptr )
				{
					Warning( "[vanish] engine recvfrom not found, query hiding is disabled\n" );
					return false;
				}

				return true;
			}

			void Detach( )
			{
				if( original == nullptr )
					return;

				Enabled( ) = false;
				if( RecvHook::Remove( ) )
				{
					original = nullptr;
					return;
				}

				// something hooked recvfrom after this module: the detour stays as
				// a plain pass through, and the module stays loaded so it can
				Warning( "[vanish] recvfrom was hooked again after vanish, keeping the module loaded\n" );
				Memory::PinModule( );
			}
		}
	}
}
