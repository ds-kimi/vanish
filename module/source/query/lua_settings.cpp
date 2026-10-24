#include "lua_api.hpp"
#include "replies.hpp"
#include "roster.hpp"
#include "state.hpp"

#include "../common/lua_args.hpp"

namespace Vanish
{
	namespace Query
	{
		namespace LuaAPI
		{
			/// vanishquery.SetEnabled( enabled )
			LUA_FUNCTION_STATIC( SetEnabled )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::BOOL );

				const bool enabled = LUA->GetBool( 1 );

				// the map, gamemode, version and port can have changed since the
				// last time replies were sent, so they are re-read when turning on
				if( enabled )
					Replies::Refresh( );

				Enabled( ) = enabled;
				return 0;
			}

			/// vanishquery.SetSecure( secure )
			LUA_FUNCTION_STATIC( SetSecure )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::BOOL );

				Roster::SetSecure( LUA->GetBool( 1 ) );
				return 0;
			}

			/// vanishquery.SetConVars( { sv_visiblemaxplayers = n, sv_location = s,
			///     sv_max_queries_sec = n, sv_max_queries_window = n,
			///     sv_max_queries_sec_global = n } )
			LUA_FUNCTION_STATIC( SetConVars )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::TABLE );

				ConVars settings;
				settings.visible_max_players = LuaArgs::ClampInteger( LuaArgs::RawNumber(
					LUA, 1, "sv_visiblemaxplayers", settings.visible_max_players ), -1, INT32_MAX );
				settings.queries_per_second = LuaArgs::ClampFloat( LuaArgs::RawNumber(
					LUA, 1, "sv_max_queries_sec", settings.queries_per_second ), 0.0f, 1e9f );
				settings.queries_window = LuaArgs::ClampFloat( LuaArgs::RawNumber(
					LUA, 1, "sv_max_queries_window", settings.queries_window ), 0.0f, 1e9f );
				settings.queries_global = LuaArgs::ClampFloat( LuaArgs::RawNumber(
					LUA, 1, "sv_max_queries_sec_global", settings.queries_global ), 0.0f, 1e9f );
				LuaArgs::RawString( LUA, 1, "sv_location", settings.location );

				std::lock_guard<std::mutex> guard( Lock( ) );
				Get( ).settings = std::move( settings );
				return 0;
			}

			void RegisterSettings( GarrysMod::Lua::ILuaBase *LUA )
			{
				LUA->PushCFunction( SetEnabled );
				LUA->SetField( -2, "SetEnabled" );

				LUA->PushCFunction( SetSecure );
				LUA->SetField( -2, "SetSecure" );

				LUA->PushCFunction( SetConVars );
				LUA->SetField( -2, "SetConVars" );
			}
		}
	}
}
