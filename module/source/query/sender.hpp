#pragma once

#include "recv_hook.hpp"

#include <cstddef>
#include <cstdint>

namespace Vanish
{
	namespace Query
	{
		/// Sends a connectionless reply the way the engine's NET_SendPacket does.
		///
		/// A reply that fits MAX_ROUTABLE_PAYLOAD goes out as one datagram.
		/// Anything longer, which A2S_PLAYER is on a full server, is split the
		/// way NET_SendLong splits it: a -2 header, a sequence number, the
		/// packet number and count, and the split size, then up to 1248 bytes
		/// of the reply. Sending the long reply whole would exceed what many
		/// routes deliver, and dropping it, as an earlier version did, left
		/// query tools with no player list at all on a busy server.
		namespace Sender
		{
			void Send( RecvHook::Socket socket, const uint8_t *data, size_t size,
				const sockaddr *to, RecvHook::AddressLength tolen );
		}
	}
}
