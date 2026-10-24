#include "game_socket.hpp"

#include <atomic>

#if defined SYSTEM_WINDOWS
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace Vanish
{
	namespace Query
	{
		namespace GameSocket
		{
			static std::atomic<uint16_t> game_port( 0 );

			// the socket last found on the game port, so only a query arriving
			// on some other socket costs a getsockname call
			static std::atomic<RecvHook::Socket> known( static_cast<RecvHook::Socket>( -1 ) );

			void SetPort( uint16_t port )
			{
				game_port = port;
				known = static_cast<RecvHook::Socket>( -1 );
			}

			bool Is( RecvHook::Socket socket )
			{
				const uint16_t port = game_port;
				if( port == 0 )
					return false;

				if( socket == known )
					return true;

				sockaddr_in local = { };
				RecvHook::AddressLength size = sizeof( local );
				if( getsockname( socket, reinterpret_cast<sockaddr *>( &local ), &size ) != 0 ||
					local.sin_family != AF_INET || ntohs( local.sin_port ) != port )
					return false;

				known = socket;
				return true;
			}
		}
	}
}
