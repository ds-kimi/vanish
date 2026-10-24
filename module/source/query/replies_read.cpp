#include "replies_cache.hpp"
#include "gamemodes.hpp"

#include <eiface.h>
#include <GarrysMod/Gamemode.h>

namespace Vanish
{
	namespace Query
	{
		namespace Replies
		{
			std::string ReadGameDirectory( IVEngineServer *engine )
			{
				char path[256] = { 0 };
				engine->GetGameDir( path, sizeof( path ) );
				path[sizeof( path ) - 1] = 0;

				std::string directory( path );
				const size_t slash = directory.find_last_of( "\\/" );
				if( slash != std::string::npos )
					directory.erase( 0, slash + 1 );

				return directory;
			}

			///@return the gamemode name the way the engine tags it
			static std::string TagName( std::string name )
			{
				// the engine strips this suffix before tagging
				const std::string suffix = "_modded";
				for( size_t at = name.find( suffix ); at != std::string::npos; at = name.find( suffix ) )
					name.erase( at, suffix.size( ) );

				return name;
			}

			///@return the gm, gmws and gmc tags of the active gamemode
			static std::string GamemodeTags( const IGamemodeSystem::Information &gamemode )
			{
				std::string tags = " gm:";
				tags += TagName( gamemode.name );

				if( gamemode.workshopid != 0 )
				{
					tags += " gmws:";
					tags += std::to_string( gamemode.workshopid );
				}

				if( !gamemode.category.empty( ) )
				{
					tags += " gmc:";
					tags += gamemode.category;
				}

				return tags;
			}

			std::string ReadGameTags( IFileSystem *filesystem, const std::string &location, int32_t version )
			{
				const IGamemodeSystem::Information *gamemode = Gamemodes::Active( filesystem );
				if( gamemode == nullptr )
					return std::string( );

				std::string tags = GamemodeTags( *gamemode );

				// the engine shows at most the first five characters, and only
				// once more than one is set
				if( location.size( ) > 1 )
				{
					tags += " loc:";
					tags += location.substr( 0, 5 );
				}

				if( version != 0 )
				{
					tags += " ver:";
					tags += std::to_string( version );
				}

				return tags;
			}
		}
	}
}
