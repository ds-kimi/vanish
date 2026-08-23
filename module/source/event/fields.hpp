#pragma once

#include <GarrysMod/Lua/Interface.h>

class IGameEvent;

namespace Vanish
{
	namespace Event
	{
		/// Copies every string keyed entry of the Lua table at iStackPos into
		/// the event, mapping Lua types onto the event key types.
		///@param LUA lua state holding the table
		///@param event event to fill
		///@param iStackPos absolute stack position of the source table
		void ReadFields( GarrysMod::Lua::ILuaBase *LUA, IGameEvent *event, int iStackPos );
	}
}
