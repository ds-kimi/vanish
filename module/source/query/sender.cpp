#include "sender.hpp"

#include <cstring>

namespace Vanish
{
	namespace Query
	{
		namespace Sender
		{
			// net_ws.cpp: MAX_ROUTABLE_PAYLOAD, the most a connectionless
			// packet carries before NET_SendLong splits it
			const size_t MAX_ROUTABLE = 1260;

			// net_ws.cpp: SPLITPACKET { int netID; int sequenceNumber;
			// short packetID; short nSplitSize; }
			const int32_t SPLIT_HEADER = -2;
			const size_t SPLIT_HEADER_SIZE = 12;
			const size_t SPLIT_PAYLOAD = MAX_ROUTABLE - SPLIT_HEADER_SIZE;

			// the packet number travels in the high byte of packetID
			const size_t MAX_SPLITS = 255;

			static void SendTo( RecvHook::Socket socket, const uint8_t *data, size_t size,
				const sockaddr *to, RecvHook::AddressLength tolen )
			{
				// a failed send is a dropped UDP reply, which the client retries
				sendto( socket, reinterpret_cast<const char *>( data ), static_cast<int>( size ), 0, to, tolen );
			}

			static void SendSplit( RecvHook::Socket socket, const uint8_t *data, size_t size,
				const sockaddr *to, RecvHook::AddressLength tolen )
			{
				// one sequence per reply, like the engine's per socket counter
				static int32_t sequence = 1;
				++sequence;

				const size_t count = ( size + SPLIT_PAYLOAD - 1 ) / SPLIT_PAYLOAD;
				uint8_t packet[MAX_ROUTABLE];
				for( size_t number = 0; number < count; ++number )
				{
					const size_t offset = number * SPLIT_PAYLOAD;
					const size_t chunk = size - offset < SPLIT_PAYLOAD ? size - offset : SPLIT_PAYLOAD;
					const uint16_t id = static_cast<uint16_t>( ( number << 8 ) | count );
					const uint16_t split = static_cast<uint16_t>( SPLIT_PAYLOAD );

					std::memcpy( packet, &SPLIT_HEADER, 4 );
					std::memcpy( packet + 4, &sequence, 4 );
					std::memcpy( packet + 8, &id, 2 );
					std::memcpy( packet + 10, &split, 2 );
					std::memcpy( packet + SPLIT_HEADER_SIZE, data + offset, chunk );
					SendTo( socket, packet, SPLIT_HEADER_SIZE + chunk, to, tolen );
				}
			}

			void Send( RecvHook::Socket socket, const uint8_t *data, size_t size,
				const sockaddr *to, RecvHook::AddressLength tolen )
			{
				if( size <= MAX_ROUTABLE )
					SendTo( socket, data, size, to, tolen );
				else if( ( size + SPLIT_PAYLOAD - 1 ) / SPLIT_PAYLOAD <= MAX_SPLITS )
					SendSplit( socket, data, size, to, tolen );
			}
		}
	}
}
