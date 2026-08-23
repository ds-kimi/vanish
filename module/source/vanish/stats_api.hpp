#pragma once

#include <GarrysMod/Lua/Interface.h>

namespace Vanish
{
	namespace Stats
	{
		/// Adds the stats readers to the table on top of the Lua stack.
		void Register( GarrysMod::Lua::ILuaBase *LUA );
	}
}
