#include "roster.hpp"
#include "state.hpp"

#include <cstring>

namespace Vanish
{
	namespace Query
	{
		namespace Roster
		{
			void SetCounts( int32_t count, int32_t bots )
			{
				const int32_t limit = static_cast<int32_t>( MAX_PLAYERS );
				const int32_t players = count < 0 ? 0 : ( count > limit ? limit : count );

				std::lock_guard<std::mutex> guard( Lock( ) );
				Get( ).visible_count = players;
				Get( ).visible_bots = bots < 0 ? 0 : ( bots > players ? players : bots );
			}

			void Clear( )
			{
				std::lock_guard<std::mutex> guard( Lock( ) );
				Get( ).players.clear( );
			}

			void Add( const char *name, size_t length, int32_t score, float time )
			{
				if( name == nullptr )
					return;

				// the reply carries the name as a C string, so it ends at the first
				// NUL anyway, and a cut must not split a UTF-8 sequence in half
				const void *terminator = std::memchr( name, 0, length );
				if( terminator != nullptr )
					length = static_cast<size_t>( static_cast<const char *>( terminator ) - name );

				if( length > MAX_NAME - 1 )
					for( length = MAX_NAME - 1; length > 0 && ( name[length] & 0xC0 ) == 0x80; --length );

				PlayerEntry entry = { std::string( name, length ), score, time };

				std::lock_guard<std::mutex> guard( Lock( ) );
				std::vector<PlayerEntry> &players = Get( ).players;
				if( players.size( ) < MAX_PLAYERS )
					players.push_back( std::move( entry ) );
			}

			void SetSecure( bool secure )
			{
				std::lock_guard<std::mutex> guard( Lock( ) );
				Get( ).secure = secure;
			}
		}
	}
}
