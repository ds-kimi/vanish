#include "challenge.hpp"

#include <Platform.hpp>

#include <chrono>
#include <random>

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
		namespace Challenge
		{
			// -1 asks for a challenge, so it can never be one
			const int32_t REQUEST = -1;

			static uint32_t secret = 0;

			void Initialize( )
			{
				if( secret != 0 )
					return;

				// random_device may throw where no entropy source is available;
				// the clock is a weaker seed, but a query must never crash the server
				try
				{
					secret = std::random_device( )( );
				}
				catch( ... )
				{
					secret = static_cast<uint32_t>(
						std::chrono::steady_clock::now( ).time_since_epoch( ).count( ) );
				}

				secret |= 1;
			}

			uint32_t Mix( uint32_t value )
			{
				// murmur3's finaliser, enough to spread the address over all 32
				// bits without carrying a hash dependency
				value ^= secret;
				value ^= value >> 16;
				value *= 0x85EBCA6BU;
				value ^= value >> 13;
				value *= 0xC2B2AE35U;
				value ^= value >> 16;
				return value;
			}

			int32_t For( const sockaddr *from )
			{
				const uint32_t address = from != nullptr && from->sa_family == AF_INET
					? reinterpret_cast<const sockaddr_in *>( from )->sin_addr.s_addr
					: 0;

				const int32_t challenge = static_cast<int32_t>( Mix( address ) );
				return challenge == REQUEST ? 0 : challenge;
			}
		}
	}
}
