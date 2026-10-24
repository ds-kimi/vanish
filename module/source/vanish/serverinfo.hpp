#pragma once

#include <GarrysMod/Lua/Interface.h>

namespace Vanish
{
	/// The vanishinfo table: values the engine prints but Lua cannot read.
	namespace ServerInfo
	{
		void Initialize( GarrysMod::Lua::ILuaBase *LUA );

		void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );

		/// Adds the Steam readers to the table on top of the Lua stack.
		void RegisterSteam( GarrysMod::Lua::ILuaBase *LUA );

		/// Adds the version reader to the table on top of the Lua stack.
		void RegisterVersion( GarrysMod::Lua::ILuaBase *LUA );
	}
}
