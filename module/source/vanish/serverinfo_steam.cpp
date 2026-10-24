#include "serverinfo.hpp"

#include <GarrysMod/InterfacePointers.hpp>

#include <steam/steamclientpublic.h>
#include <steam/steam_gameserver.h>
#include <eiface.h>

#include <cstdint>
#include <cstdio>

namespace Vanish
{
	namespace ServerInfo
	{
		/// Pushes the game server SteamID as a string, since Lua numbers lose
		/// precision above 2^53 and every caller only formats it anyway. Nil
		/// until Steam is logged on, which is when status says "not logged in".
		LUA_FUNCTION_STATIC( GetSteamID64 )
		{
			IVEngineServer *engine = InterfacePointers::VEngineServer( );
			ISteamGameServer *steam = SteamGameServer( );
			const bool logged_on = steam != nullptr && steam->BLoggedOn( );
			const CSteamID *steamid = logged_on && engine != nullptr
				? engine->GetGameServerSteamID( ) : nullptr;

			if( steamid == nullptr )
			{
				LUA->PushNil( );
				return 1;
			}

			// uint64 is unsigned long on 64-bit Linux, so the cast keeps %llu
			// matching its argument on every build
			char text[32] = { 0 };
			std::snprintf( text, sizeof( text ), "%llu",
				static_cast<unsigned long long>( steamid->ConvertToUint64( ) ) );

			LUA->PushString( text );
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
			std::snprintf( text, sizeof( text ), "%u.%u.%u.%u",
				( ip >> 24 ) & 0xFF, ( ip >> 16 ) & 0xFF, ( ip >> 8 ) & 0xFF, ip & 0xFF );

			LUA->PushString( text );
			return 1;
		}

		void RegisterSteam( GarrysMod::Lua::ILuaBase *LUA )
		{
			LUA->PushCFunction( GetSteamID64 );
			LUA->SetField( -2, "GetSteamID64" );

			LUA->PushCFunction( IsSecure );
			LUA->SetField( -2, "IsSecure" );

			LUA->PushCFunction( GetPublicIP );
			LUA->SetField( -2, "GetPublicIP" );
		}
	}
}
