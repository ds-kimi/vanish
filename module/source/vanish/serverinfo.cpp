#include "serverinfo.hpp"

#include <GarrysMod/InterfacePointers.hpp>

#include <steam/steamclientpublic.h>
#include <eiface.h>

namespace Vanish
{
namespace ServerInfo
{
	static IVEngineServer *engine_server = nullptr;

	/// Pushes the game server SteamID as a string, since Lua numbers lose
	/// precision above 2^53 and every caller only formats it anyway.
	LUA_FUNCTION_STATIC( GetSteamID64 )
	{
		if( engine_server == nullptr )
			engine_server = InterfacePointers::VEngineServer( );

		const CSteamID *steamid =
			engine_server != nullptr ? engine_server->GetGameServerSteamID( ) : nullptr;

		if( steamid == nullptr )
		{
			LUA->PushNil( );
			return 1;
		}

		char text[32] = { 0 };
		V_snprintf( text, sizeof( text ), "%llu", steamid->ConvertToUint64( ) );

		LUA->PushString( text );
		return 1;
	}

	void Initialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->CreateTable( );

		LUA->PushCFunction( GetSteamID64 );
		LUA->SetField( -2, "GetSteamID64" );

		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishinfo" );
	}

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishinfo" );
	}
}
}
