// Derived from gmsv_spoof by Daniel Almeida (BSD 3-Clause), see
// licenses/gmsv_spoof.txt. The reply layout is the Source A2S format; the
// signature scans, threaded socket and firewall of the original are gone,
// every interface here is resolved through garrysmod_common instead.

#include "replies.hpp"
#include "replies_cache.hpp"
#include "game_socket.hpp"
#include "state.hpp"

#include "../vanish/game_server.hpp"

#include <GarrysMod/InterfacePointers.hpp>

#include <eiface.h>
#include <iserver.h>

namespace Vanish
{
	namespace Query
	{
		namespace Replies
		{
			bool Initialize( )
			{
				Interfaces &engine = Engine( );
				engine.server = GameServer::Get( );
				engine.engine = InterfacePointers::VEngineServer( );
				engine.gamedll = InterfacePointers::ServerGameDLL( );
				engine.filesystem = InterfacePointers::FileSystemServer( );

				return engine.server != nullptr && engine.engine != nullptr &&
					engine.gamedll != nullptr && engine.filesystem != nullptr;
			}

			void SetVersion( int32_t version )
			{
				std::lock_guard<std::mutex> guard( Lock( ) );
				Data( ).lua_version = version;
			}

			void Refresh( )
			{
				const Interfaces &engine = Engine( );
				if( engine.server == nullptr )
					return;

				std::string location;
				int32_t version = 0;
				{
					std::lock_guard<std::mutex> guard( Lock( ) );
					location = Get( ).settings.location;
					version = Data( ).lua_version;
				}

				// read without the lock: these touch the engine and the disk
				Cache fresh;
				fresh.directory = ReadGameDirectory( engine.engine );
				fresh.tags = ReadGameTags( engine.filesystem, location, version );
				fresh.version = ReadGameVersion( engine.filesystem );
				fresh.max_clients = engine.server->GetMaxClients( );
				fresh.lua_version = version;

				GameSocket::SetPort( static_cast<uint16_t>( engine.server->GetUDPPort( ) ) );

				std::lock_guard<std::mutex> guard( Lock( ) );
				Data( ) = std::move( fresh );
			}
		}
	}
}
