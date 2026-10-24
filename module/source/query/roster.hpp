#pragma once

#include <cstddef>
#include <cstdint>

namespace Vanish
{
	namespace Query
	{
		/// Writes into the query state, each taking the lock for itself.
		namespace Roster
		{
			/// Sets the counts A2S_INFO reports. Both come from the masked
			/// roster: a vanished bot left in the engine's own bot count would
			/// report more bots than players.
			void SetCounts( int32_t count, int32_t bots );

			void Clear( );

			/// Adds one row to A2S_PLAYER, silently ignored past MAX_PLAYERS:
			/// a reply missing a row is better than one that is not sent.
			///@param name bytes of the name, not necessarily NUL terminated
			///@param length byte count, truncated to fit MAX_NAME
			void Add( const char *name, size_t length, int32_t score, float time );

			void SetSecure( bool secure );
		}
	}
}
