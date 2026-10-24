#pragma once

#include "recv_hook.hpp"

#include <cstddef>

namespace Vanish
{
	namespace Query
	{
		/// Answers one A2S_INFO or A2S_PLAYER query in the engine's place.
		namespace Answer
		{
			/// Rate limits the sender, then replies with a challenge or with
			/// the filtered reply. Never throws: it runs inside the engine's
			/// read path, where an escaping exception would end the process.
			void Query( RecvHook::Socket socket, const sockaddr *from, RecvHook::AddressLength fromlen,
				char type, const char *data, size_t size );
		}
	}
}
