#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Vanish
{
	namespace Query
	{
		// A2S_PLAYER has to fit one datagram, and a Source server never has
		// more slots than this anyway.
		const size_t MAX_PLAYERS = 128;

		struct PlayerEntry
		{
			std::string name;
			int32_t score;
			float time;
		};

		/// The roster and flags the query replies are built from.
		///
		/// Lua fills this with the players an outside observer is allowed to
		/// see; the socket filter never looks at the real player list.
		class State
		{
		public:
			State( );

			bool IsEnabled( ) const;
			void SetEnabled( bool enabled );

			bool IsSecure( ) const;
			void SetSecure( bool secure );

			int32_t VisibleCount( ) const;
			void SetVisibleCount( int32_t count );

			void ClearPlayers( );
			void AddPlayer( const char *name, int32_t score, float time );

			const std::vector<PlayerEntry> &Players( ) const;

		private:
			bool m_enabled;

			// the engine cannot report VAC state to a spoofed reply on every
			// build, so it is asserted from Lua instead of guessed
			bool m_secure;

			int32_t m_visible_count;
			std::vector<PlayerEntry> m_players;
		};

		State &Get( );
	}
}
