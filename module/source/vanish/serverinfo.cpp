#include "serverinfo.hpp"
#include "stats_api.hpp"

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

	/// Pushes the engine build number the status line prints beside the patch
	/// version. A dedicated server's steam.inf carries only PatchVersion, so
	/// this is the only place the real number can come from.
	LUA_FUNCTION_STATIC( GetServerVersion )
	{
		if( engine_server == nullptr )
			engine_server = InterfacePointers::VEngineServer( );

		const int version =
			engine_server != nullptr ? engine_server->GetServerVersion( ) : 0;

		if( version <= 0 )
		{
			LUA->PushNil( );
			return 1;
		}

		LUA->PushNumber( version );
		return 1;
	}

	void Initialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->CreateTable( );

		LUA->PushCFunction( GetSteamID64 );
		LUA->SetField( -2, "GetSteamID64" );

		LUA->PushCFunction( GetServerVersion );
		LUA->SetField( -2, "GetServerVersion" );

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
