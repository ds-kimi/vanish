#pragma once

#include <cstdint>

class CCheckTransmitInfo;
class IVEngineServer;

namespace Vanish
{
	/// The per client work of the CheckTransmit hook.
	namespace TransmitStrip
	{
		///@return entity index of the receiving client, 0 when unresolved
		int32_t ReceiverOf( IVEngineServer *engine, const CCheckTransmitInfo *info );

		/// Removes every hidden entity from this client's transmit sets,
		/// except the receiver's own entity.
		void StripHidden( CCheckTransmitInfo *info, int32_t receiver );
	}
}
