#pragma once

#include <cstdint>

class INetworkStringTable;

namespace Vanish
{
	namespace UserInfo
	{
		/// Access to the userinfo string table, looked up on every use.
		///
		/// The table is created per map and destroyed at level shutdown, so a
		/// pointer kept across a map change would point into freed memory.
		namespace Entry
		{
			///@return the userinfo table, nullptr when it does not exist right now
			INetworkStringTable *Table( );

			///@return the user data of slot, nullptr when the slot is out of range
			///or carries none
			const uint8_t *Data( INetworkStringTable *table, int32_t slot, int32_t &length );

			///@return whether every byte is zero, which is what Hide writes
			bool IsBlank( const uint8_t *data, int32_t length );
		}
	}
}
