#include "serverinfo.hpp"
#include "stats_api.hpp"

namespace Vanish
{
	namespace ServerInfo
	{
		void Initialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			LUA->CreateTable( );

			RegisterSteam( LUA );
			RegisterVersion( LUA );
			Stats::Register( LUA );

			LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishinfo" );
		}

		void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			LUA->PushNil( );
			LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishinfo" );
		}
	}
}
