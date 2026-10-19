#include "challenge.hpp"

#include <Platform.hpp>

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

			///@return a secret drawn once per process, so challenges cannot
			///be predicted from the address alone
			static uint32_t Secret( )
			{
				static const uint32_t secret = std::random_device( )( );
				return secret;
			}

			///@return the sender's IPv4 address, 0 for any other family
			static uint32_t Address( const sockaddr *from )
			{
				if( from == nullptr || from->sa_family != AF_INET )
					return 0;

				return reinterpret_cast<const sockaddr_in *>( from )->sin_addr.s_addr;
			}

			int32_t For( const sockaddr *from )
			{
				// murmur3's finaliser, enough to spread the address over all
				// 32 bits without carrying a hash dependency
				uint32_t value = Address( from ) ^ Secret( );
				value ^= value >> 16;
				value *= 0x85EBCA6BU;
				value ^= value >> 13;
				value *= 0xC2B2AE35U;
				value ^= value >> 16;

				const int32_t challenge = static_cast<int32_t>( value );
				return challenge == REQUEST ? 0 : challenge;
			}
		}
	}
}
