#pragma once

#include <cstdint>

class bf_write;

namespace Vanish
{
	namespace Query
	{
		/// Builds the A2S_INFO and A2S_PLAYER replies the filter sends back.
		///
		/// Everything that cannot change while the map is running is read once
		/// through Refresh, so answering a query costs no file access.
		namespace Replies
		{
			///@return true when every interface the replies need was resolved
			bool Initialize( );

			/// Sets the game's VERSION number, advertised in the ver: tag.
			void SetVersion( int32_t version );

			/// Re-reads the tags, version, slot count and port. Takes the
			/// query state lock itself.
			void Refresh( );

			/// Both builders must be called with the query state lock held.
			void BuildInfo( bf_write &out );

			void BuildPlayers( bf_write &out );
		}
	}
}
