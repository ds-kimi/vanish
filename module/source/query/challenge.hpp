#pragma once

#include <cstdint>

struct sockaddr;

namespace Vanish
{
	namespace Query
	{
		/// The challenge numbers A2S_INFO and A2S_PLAYER replies are gated on.
		///
		/// The engine answers a query without a valid challenge with an 'A'
		/// packet carrying one, and only answers for real once the client
		/// repeats the query with it. Answering straight away would make the
		/// filtered replies distinguishable from the engine's, and would hand
		/// a reflection amplifier to anyone spoofing a source address.
		namespace Challenge
		{
			///@return the challenge this address has to echo back
			int32_t For( const sockaddr *from );
		}
	}
}
