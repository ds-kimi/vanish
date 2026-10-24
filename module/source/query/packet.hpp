#pragma once

#include <cstddef>
#include <cstdint>

struct sockaddr;

namespace Vanish
{
	namespace Query
	{
		/// Reads the connectionless datagrams the filter answers. Everything
		/// here reads attacker supplied bytes, so every read is bounds checked.
		namespace Packet
		{
			const char A2S_INFO = 'T';
			const char A2S_PLAYER = 'U';

			///@return A2S_INFO or A2S_PLAYER, 0 for anything else
			char QueryType( const char *data, size_t size );

			///@return whether the sender is a loopback address, which the
			///engine never answers queries from
			bool FromLoopback( const sockaddr *from );

			///@param challenge filled with the challenge the query echoes
			///@return false when the query is too short to carry one
			bool EchoedChallenge( const char *data, size_t size, char type, int32_t &challenge );
		}
	}
}
