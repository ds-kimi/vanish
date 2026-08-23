#pragma once

#include <GarrysMod/Lua/Interface.h>

namespace Vanish
{
	/// The transmitcontrol table the addon drives the hidden set through.
	namespace LuaAPI
	{
		void Initialize( GarrysMod::Lua::ILuaBase *LUA );

		void Deinitialize( GarrysMod::Lua::ILuaBase *LUA );
	}
}
