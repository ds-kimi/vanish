#include "serverinfo.hpp"
#include "stats_api.hpp"

#include "../main.hpp"

#include <GarrysMod/InterfacePointers.hpp>
#include <Platform.hpp>
#include <scanning/symbolfinder.hpp>

#include <steam/steamclientpublic.h>
#include <steam/steam_gameserver.h>
#include <eiface.h>

#include <cstdint>
#include <cstring>

namespace Vanish
{
namespace ServerInfo
{
	static IVEngineServer *engine_server = nullptr;

	/// Pushes the game server SteamID as a string, since Lua numbers lose
	/// precision above 2^53 and every caller only formats it anyway. Nil
	/// until Steam is logged on, which is when status says "not logged in".
	LUA_FUNCTION_STATIC( GetSteamID64 )
	{
		if( engine_server == nullptr )
			engine_server = InterfacePointers::VEngineServer( );

		ISteamGameServer *steam = SteamGameServer( );
		const bool logged_on = steam != nullptr && steam->BLoggedOn( );

		const CSteamID *steamid = logged_on && engine_server != nullptr
			? engine_server->GetGameServerSteamID( ) : nullptr;

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

#if defined SYSTEM_LINUX

	// The status command prints build_number( ), which the Linux engine does
	// not export. GetServerVersion there reads a ServerVersion key that a
	// dedicated server's steam.inf does not carry, so it returns 0. The build
	// is taken from the one call to build_number( ) inside status instead:
	//
	//   call build_number
	//   mov  edx, [ebp-0x2c8]
	//   mov  [esp+0x14], edi
	//   mov  [esp+0xc], eax          the build, the second %d of the version line
	static const char BUILD_NUMBER_CALL[] =
		"\xE8\x2A\x2A\x2A\x2A\x8B\x95\x2A\x2A\x2A\x2A\x89\x7C\x24\x14\x89\x44\x24\x0C";

	// push ebp; mov eax, [build]; mov ebp, esp; pop ebp; ret
	static const uint8_t BUILD_NUMBER_BODY[] = { 0x55, 0xA1 };
	static const uint8_t BUILD_NUMBER_TAIL[] = { 0x89, 0xE5, 0x5D, 0xC3 };

	///@return the engine's build number, 0 when it cannot be found
	static int FindBuildNumber( )
	{
		SymbolFinder finder;
		const uint8_t *call = static_cast<const uint8_t *>( finder.Resolve(
			global::engine_loader.GetModule( ), BUILD_NUMBER_CALL, sizeof( BUILD_NUMBER_CALL ) - 1 ) );

		if( call == nullptr )
			return 0;

		int32_t relative = 0;
		std::memcpy( &relative, call + 1, sizeof( relative ) );
		const uint8_t *function = call + 5 + relative;

		// only a plain getter is called: anything else means the signature
		// landed somewhere it was not written for
		if( std::memcmp( function, BUILD_NUMBER_BODY, sizeof( BUILD_NUMBER_BODY ) ) != 0 ||
			std::memcmp( function + 6, BUILD_NUMBER_TAIL, sizeof( BUILD_NUMBER_TAIL ) ) != 0 )
			return 0;

		return reinterpret_cast<int ( * )( )>( const_cast<uint8_t *>( function ) )( );
	}

	///@return the build number, scanned for once since it never changes
	static int BuildNumber( )
	{
		static const int build = FindBuildNumber( );
		return build;
	}

#endif

	/// Pushes the engine build number the status line prints beside the patch
	/// version. A dedicated server's steam.inf carries only PatchVersion, so
	/// this is the only place the real number can come from.
	LUA_FUNCTION_STATIC( GetServerVersion )
	{
		if( engine_server == nullptr )
			engine_server = InterfacePointers::VEngineServer( );

		int version =
			engine_server != nullptr ? engine_server->GetServerVersion( ) : 0;

#if defined SYSTEM_LINUX

		if( version <= 0 )
			version = BuildNumber( );

#endif

		if( version <= 0 )
		{
			LUA->PushNil( );
			return 1;
		}

		LUA->PushNumber( version );
		return 1;
	}

	/// Pushes whether VAC is protecting this session, which the status line
	/// and the A2S reply both report, or nil before Steam is up.
	LUA_FUNCTION_STATIC( IsSecure )
	{
		ISteamGameServer *steam = SteamGameServer( );
		if( steam == nullptr )
		{
			LUA->PushNil( );
			return 1;
		}

		LUA->PushBool( steam->BSecure( ) );
		return 1;
	}

	/// Pushes the public address Steam sees, which status prints beside the
	/// bound one, or nil while Steam does not know it.
	LUA_FUNCTION_STATIC( GetPublicIP )
	{
		ISteamGameServer *steam = SteamGameServer( );
		const SteamIPAddress_t address =
			steam != nullptr ? steam->GetPublicIP( ) : SteamIPAddress_t( );

		if( !address.IsSet( ) || address.m_eType != k_ESteamIPTypeIPv4 )
		{
			LUA->PushNil( );
			return 1;
		}

		const uint32_t ip = address.m_unIPv4;
		char text[16] = { 0 };
		V_snprintf( text, sizeof( text ), "%u.%u.%u.%u",
			( ip >> 24 ) & 0xFF, ( ip >> 16 ) & 0xFF, ( ip >> 8 ) & 0xFF, ip & 0xFF );

		LUA->PushString( text );
		return 1;
	}

	void Initialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->CreateTable( );

		LUA->PushCFunction( GetSteamID64 );
		LUA->SetField( -2, "GetSteamID64" );

		LUA->PushCFunction( GetServerVersion );
		LUA->SetField( -2, "GetServerVersion" );

		LUA->PushCFunction( IsSecure );
		LUA->SetField( -2, "IsSecure" );

		LUA->PushCFunction( GetPublicIP );
		LUA->SetField( -2, "GetPublicIP" );

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
