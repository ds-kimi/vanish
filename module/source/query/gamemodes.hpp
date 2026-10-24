#pragma once

#include <GarrysMod/Gamemode.h>

class IFileSystem;

namespace Vanish
{
	namespace Query
	{
		/// The engine's gamemode system, which carries the gm: tags.
		namespace Gamemodes
		{
			///@return the active gamemode, nullptr when the system is unreachable
			const IGamemodeSystem::Information *Active( IFileSystem *filesystem );
		}
	}
}
