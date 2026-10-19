#include "state.hpp"

namespace Vanish
{
	namespace Query
	{
		State::State( ) :
			m_enabled( false ),
			m_secure( true ),
			m_visible_count( 0 ),
			m_visible_bots( 0 )
		{
			m_players.reserve( MAX_PLAYERS );
		}

		bool State::IsEnabled( ) const
		{
			return m_enabled;
		}

		void State::SetEnabled( bool enabled )
		{
			m_enabled = enabled;
		}

		bool State::IsSecure( ) const
		{
			return m_secure;
		}

		void State::SetSecure( bool secure )
		{
			m_secure = secure;
		}

		int32_t State::VisibleCount( ) const
		{
			return m_visible_count;
		}

		int32_t State::VisibleBots( ) const
		{
			return m_visible_bots;
		}

		void State::SetVisibleCount( int32_t count, int32_t bots )
		{
			m_visible_count = count < 0 ? 0 : count;
			m_visible_bots = bots < 0 ? 0 : ( bots > m_visible_count ? m_visible_count : bots );
		}

		void State::ClearPlayers( )
		{
			m_players.clear( );
		}

		void State::AddPlayer( const char *name, int32_t score, float time )
		{
			// silently ignored past the cap: a reply that does not fit the
			// datagram is worse than a short one
			if( name == nullptr || m_players.size( ) >= MAX_PLAYERS )
				return;

			PlayerEntry entry;
			entry.name = name;
			entry.score = score;
			entry.time = time;

			m_players.push_back( entry );
		}

		const std::vector<PlayerEntry> &State::Players( ) const
		{
			return m_players;
		}

		const ConVars &State::Settings( ) const
		{
			return m_settings;
		}

		void State::SetSettings( const ConVars &settings )
		{
			m_settings = settings;
		}

		State &Get( )
		{
			static State state;
			return state;
		}
	}
}
