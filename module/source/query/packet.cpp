#include "packet.hpp"

#include <Platform.hpp>

#include <cstring>

#if defined SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

namespace Vanish
{
	namespace Query
	{
		namespace Packet
		{
			// connectionless packets start with this, then a one byte type
			const int32_t CONNECTIONLESS = -1;
			const size_t HEADER_SIZE = 5;

			// A2S_INFO carries this payload before its challenge, A2S_PLAYER
			// carries its challenge straight after the type byte
			const char INFO_PAYLOAD[] = "Source Engine Query";
			const size_t INFO_CHALLENGE_AT = HEADER_SIZE + sizeof( INFO_PAYLOAD );
			const size_t PLAYER_CHALLENGE_AT = HEADER_SIZE;

			char QueryType( const char *data, size_t size )
			{
				if( data == nullptr || size < HEADER_SIZE )
					return 0;

				int32_t header = 0;
				std::memcpy( &header, data, sizeof( header ) );
				if( header != CONNECTIONLESS )
					return 0;

				const char type = data[4];
				return type == A2S_INFO || type == A2S_PLAYER ? type : 0;
			}

			bool FromLoopback( const sockaddr *from )
			{
				if( from == nullptr || from->sa_family != AF_INET )
					return false;

				// 127.0.0.0/8, first byte of the network order address
				const uint8_t *address = reinterpret_cast<const uint8_t *>(
					&reinterpret_cast<const sockaddr_in *>( from )->sin_addr );
				return address[0] == 127;
			}

			bool EchoedChallenge( const char *data, size_t size, char type, int32_t &challenge )
			{
				const size_t at = type == A2S_INFO ? INFO_CHALLENGE_AT : PLAYER_CHALLENGE_AT;
				if( size < at + sizeof( challenge ) )
					return false;

				std::memcpy( &challenge, data + at, sizeof( challenge ) );
				return true;
			}
		}
	}
}
