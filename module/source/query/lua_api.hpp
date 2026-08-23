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
		}
	}
}
