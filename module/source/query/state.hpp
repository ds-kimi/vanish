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

		/// Engine convars the replies follow. Lua reads them with GetConVar
		/// and hands them over: the module's own ICvar does not resolve on the
		/// Linux engine, and calling the one vstdlib exports there crashes.
		/// Until Lua sets them, the engine's defaults apply.
		struct ConVars
		{
			int32_t visible_max_players = -1;
			std::string location;
			float queries_per_second = 3.0f;
			float queries_window = 30.0f;
			float queries_global = 3000.0f;
		};

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
			int32_t VisibleBots( ) const;

			/// Sets the counts A2S_INFO reports. Both come from the masked
			/// roster: a vanished bot left in the engine's own bot count
			/// would report more bots than players.
			void SetVisibleCount( int32_t count, int32_t bots );

			void ClearPlayers( );
			void AddPlayer( const char *name, int32_t score, float time );

			const std::vector<PlayerEntry> &Players( ) const;

			const ConVars &Settings( ) const;
			void SetSettings( const ConVars &settings );

		private:
			bool m_enabled;

			// the engine cannot report VAC state to a spoofed reply on every
			// build, so it is asserted from Lua instead of guessed
			bool m_secure;

			int32_t m_visible_count;
			int32_t m_visible_bots;
			std::vector<PlayerEntry> m_players;
			ConVars m_settings;
		};

		State &Get( );
	}
}
