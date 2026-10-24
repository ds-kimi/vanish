#include "answer.hpp"
#include "challenge.hpp"
#include "packet.hpp"
#include "rate_limit.hpp"
#include "replies.hpp"
#include "sender.hpp"
#include "state.hpp"

#include <bitbuf.h>

#if defined SYSTEM_WINDOWS
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#endif

namespace Vanish
{
	namespace Query
	{
		namespace Answer
		{
			// A2S_PLAYER at its largest: a 6 byte header, then per player an
			// index, a MAX_NAME name, a score and a time
			const size_t MAX_REPLY = 6 + MAX_PLAYERS * ( 1 + MAX_NAME + 4 + 4 );

			const char S2C_CHALLENGE = 'A';

			/// Builds the reply into out, with the state lock held.
			///@return false when it did not fit, which the bounds above rule out
			static bool Build( bf_write &out, char type, bool challenged, const sockaddr *from )
			{
				if( !challenged )
				{
					out.WriteLong( -1 );
					out.WriteByte( S2C_CHALLENGE );
					out.WriteLong( Challenge::For( from ) );
				}
				else if( type == Packet::A2S_INFO )
					Replies::BuildInfo( out );
				else
					Replies::BuildPlayers( out );

				return !out.IsOverflowed( );
			}

			static void Respond( RecvHook::Socket socket, const sockaddr *from, RecvHook::AddressLength fromlen,
				char type, bool challenged )
			{
				const uint32_t address = reinterpret_cast<const sockaddr_in *>( from )->sin_addr.s_addr;

				// sent while still holding the lock: the send is a non-blocking
				// UDP write, and the split sequence counter is shared state
				std::lock_guard<std::mutex> guard( Lock( ) );
				if( !RateLimit::Allow( address ) )
					return;

				static uint8_t buffer[MAX_REPLY];
				bf_write packet( buffer, sizeof( buffer ) );
				if( Build( packet, type, challenged, from ) )
					Sender::Send( socket, buffer, packet.GetNumBytesWritten( ), from, fromlen );
			}

			void Query( RecvHook::Socket socket, const sockaddr *from, RecvHook::AddressLength fromlen,
				char type, const char *data, size_t size )
			{
				// the engine only ever listens on IPv4
				if( from == nullptr || fromlen < static_cast<RecvHook::AddressLength>( sizeof( sockaddr_in ) ) ||
					from->sa_family != AF_INET )
					return;

				int32_t challenge = 0;
				const bool challenged = Packet::EchoedChallenge( data, size, type, challenge ) &&
					challenge == Challenge::For( from );

				try
				{
					Respond( socket, from, fromlen, type, challenged );
				}
				catch( ... )
				{
					// out of memory while tracking the sender: the query goes
					// unanswered, as it would from an engine under the same load
				}
			}
		}
	}
}
