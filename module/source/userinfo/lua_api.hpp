#pragma once

#include <GarrysMod/Lua/Interface.h>

namespace Vanish
{
	namespace UserInfo
	{
		namespace LuaAPI
		{
			void Initialize( GarrysMod::Lua::ILuaBase *LUA );
			void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
		}
	}
}
