#include "lua_api.hpp"
#include "hidden_set.hpp"

namespace Vanish
{
	namespace LuaAPI
	{
		void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			// the set outlives the Lua state when the module stays loaded across
			// a map change, and entity indexes mean nothing on the next map
			Hidden( ).Reset( );

			LUA->PushNil( );
			LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "transmitcontrol" );
		}
	}
}
