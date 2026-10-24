#pragma once

#include <cstdint>
#include <string>

class IServer;
class IVEngineServer;
class IServerGameDLL;
class IFileSystem;

namespace Vanish
{
	namespace Query
	{
		namespace Replies
		{
			/// The engine interfaces the replies read, resolved once.
			struct Interfaces
			{
				IServer *server = nullptr;
				IVEngineServer *engine = nullptr;
				IServerGameDLL *gamedll = nullptr;
				IFileSystem *filesystem = nullptr;
			};

			/// What Refresh reads, kept so a query costs no engine or file access.
			/// Only touched while holding the query state lock.
			struct Cache
			{
				std::string directory;
				std::string version;
				std::string tags;
				int32_t max_clients = 0;
				int32_t lua_version = 0;
			};

			Interfaces &Engine( );

			Cache &Data( );

			/// The readers Refresh fills the cache with, called without the lock.
			std::string ReadGameDirectory( IVEngineServer *engine );
			std::string ReadGameTags( IFileSystem *filesystem, const std::string &location, int32_t version );
			std::string ReadGameVersion( IFileSystem *filesystem );
		}
	}
}
