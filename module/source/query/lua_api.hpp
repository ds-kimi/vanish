#pragma once

#include <GarrysMod/Lua/Interface.h>

namespace Vanish
{
	namespace Query
	{
		/// The vanishquery table the addon feeds the visible roster through.
		namespace LuaAPI
		{
			void Initialize( GarrysMod::Lua::ILuaBase *LUA );

			void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );

			/// Adds the roster functions to the table on top of the stack.
			void RegisterRoster( GarrysMod::Lua::ILuaBase *LUA );

			/// Adds the settings functions to the table on top of the stack.
			void RegisterSettings( GarrysMod::Lua::ILuaBase *LUA );
		}
	}
}
