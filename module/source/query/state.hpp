#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace Vanish
{
	namespace Query
	{
		// A Source server never has more slots than this.
		const size_t MAX_PLAYERS = 128;

		// The engine's own name buffer, NUL included. Names from Lua are
		// already bounded by it; the cap only keeps a reply's size bounded no
		// matter what Lua hands over.
		const size_t MAX_NAME = 128;

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
		/// see; the socket filter never looks at the real player list. Every
		/// field is read and written under Lock( ), because the engine's read
		/// path and Lua are not guaranteed to share a thread.
		struct State
		{
			bool secure = true;
			int32_t visible_count = 0;
			int32_t visible_bots = 0;
			std::vector<PlayerEntry> players;
			ConVars settings;
		};

		std::mutex &Lock( );

		///@return the state, only to be touched while holding Lock( )
		State &Get( );

		/// Whether replies are answered here at all. Read on every datagram
		/// without the lock, so it is atomic rather than part of State.
		std::atomic<bool> &Enabled( );
	}
}
