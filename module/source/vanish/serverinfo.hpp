#pragma once

#include <GarrysMod/Lua/Interface.h>

namespace Vanish
{
	/// The vanishinfo table: values the engine prints but Lua cannot read.
	namespace ServerInfo
	{
		void Initialize( GarrysMod::Lua::ILuaBase *LUA );

		void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
	}
}
