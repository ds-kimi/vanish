// Derived from gmsv_spoof by Daniel Almeida (BSD 3-Clause), see
// licenses/gmsv_spoof.txt. Reduced to the query path: the original also
// carried a threaded socket, packet sampling, a firewall and packet
// validation, none of which this addon needs.

#include "socket_filter.hpp"
#include "replies.hpp"
#include "state.hpp"

#include <GarrysMod/FunctionPointers.hpp>

#include <tier0/dbg.h>
#include <tier0/vcrmode.h>
#include <bitbuf.h>

#include <Platform.hpp>

#if defined SYSTEM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

typedef int SOCKET;
static const SOCKET INVALID_SOCKET = -1;

#endif

namespace Vanish
{
	namespace Query
	{
		namespace SocketFilter
		{
			// engine layout of a network socket entry; only the UDP handle is
			// read, and GMOD_GetNetSocket is an exported symbol so this does
			// not depend on a signature
			struct netsocket_t
			{
				int32_t port;
				bool listening;
				int32_t udp;
				int32_t tcp;
			};

			// socket 1 is the game socket, the one clients and query tools talk to
			const int GAME_SOCKET = 1;

			// connectionless packets start with this, then a one byte type
			const int32_t CONNECTIONLESS = -1;
			const size_t HEADER_SIZE = 5;

			const char A2S_INFO = 'T';
			const char A2S_PLAYER = 'U';

			typedef int32_t ( *recvfrom_t )(
				int32_t socket, char *buffer, int32_t length, int32_t flags,
				sockaddr *from, int32_t *fromlen );

			static recvfrom_t original_recvfrom = nullptr;
			static SOCKET game_socket = INVALID_SOCKET;
			static bool attached = false;

			///@return the query this packet is asking for, 0 when it is not one
			static char QueryType( const char *data, int32_t length )
			{
				if( length < static_cast<int32_t>( HEADER_SIZE ) )
					return 0;

				if( *reinterpret_cast<const int32_t *>( data ) != CONNECTIONLESS )
					return 0;

				const char type = data[4];
				return type == A2S_INFO || type == A2S_PLAYER ? type : 0;
			}

			/// Tells the engine the socket had nothing to read, which is how a
			/// packet handled here disappears from its point of view.
			static int32_t Swallow( )
			{
#if defined SYSTEM_WINDOWS
				WSASetLastError( WSAEWOULDBLOCK );
#else
				errno = EWOULDBLOCK;
#endif
				return -1;
			}

			static void Reply( const sockaddr *to, int32_t tolen, char type )
			{
				char buffer[1400] = { 0 };
				bf_write packet( buffer, sizeof( buffer ) );

				if( type == A2S_INFO )
					Replies::BuildInfo( packet );
				else
					Replies::BuildPlayers( packet );

				if( packet.IsOverflowed( ) )
					return;

				sendto( game_socket, buffer, packet.GetNumBytesWritten( ), 0, to, tolen );
			}

			static int32_t Detour(
				int32_t socket, char *buffer, int32_t length, int32_t flags,
				sockaddr *from, int32_t *fromlen )
			{
				const int32_t received =
					original_recvfrom( socket, buffer, length, flags, from, fromlen );

				if( received <= 0 || !Get( ).IsEnabled( ) )
					return received;

				const char type = QueryType( buffer, received );
				if( type == 0 )
					return received;

				Reply( from, *fromlen, type );
				return Swallow( );
			}

			bool Attach( )
			{
				if( attached )
					return true;

				FunctionPointers::GMOD_GetNetSocket_t GetNetSocket =
					FunctionPointers::GMOD_GetNetSocket( );

				const netsocket_t *socket = GetNetSocket != nullptr
					? reinterpret_cast<const netsocket_t *>( GetNetSocket( GAME_SOCKET ) )
					: nullptr;

				if( socket == nullptr || socket->udp == INVALID_SOCKET )
				{
					Warning( "[vanish] no game socket, query hiding is disabled\n" );
					return false;
				}

				game_socket = socket->udp;

				if( !Replies::Initialize( ) )
				{
					Warning( "[vanish] missing engine interfaces, query hiding is disabled\n" );
					return false;
				}

				original_recvfrom = VCRHook_recvfrom;
				VCRHook_recvfrom = Detour;
				attached = true;

				return true;
			}

			void Detach( )
			{
				if( !attached )
					return;

				VCRHook_recvfrom = original_recvfrom;
				attached = false;
			}
		}
	}
}
