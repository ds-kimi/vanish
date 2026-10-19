#include "lua_api.hpp"
#include "state.hpp"
#include "replies.hpp"
#include "socket_filter.hpp"

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

				// the map, gamemode and version can have changed since the last
				// time replies were sent, so they are re-read when turning on
				if( enabled )
					Replies::Refresh( );

				Get( ).SetEnabled( enabled );
				return 0;
			}

			/// vanishquery.SetPlayerCount( count, bots )
			LUA_FUNCTION_STATIC( SetPlayerCount )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );
				LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

				Get( ).SetVisibleCount(
					static_cast<int32_t>( LUA->GetNumber( 1 ) ),
					static_cast<int32_t>( LUA->GetNumber( 2 ) ) );

				return 0;
			}

			/// vanishquery.ResetPlayers( )
			LUA_FUNCTION_STATIC( ResetPlayers )
			{
				Get( ).ClearPlayers( );
				return 0;
			}

			/// vanishquery.AddPlayer( name, score, seconds )
			LUA_FUNCTION_STATIC( AddPlayer )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::STRING );
				LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
				LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

				Get( ).AddPlayer(
					LUA->GetString( 1 ),
					static_cast<int32_t>( LUA->GetNumber( 2 ) ),
					static_cast<float>( LUA->GetNumber( 3 ) ) );

				return 0;
			}

			///@return number field of the table at index 1, fallback when absent
			static double NumberField( GarrysMod::Lua::ILuaBase *LUA, const char *name, double fallback )
			{
				LUA->GetField( 1, name );
				const double value = LUA->IsType( -1, GarrysMod::Lua::Type::NUMBER )
					? LUA->GetNumber( -1 ) : fallback;

				LUA->Pop( 1 );
				return value;
			}

			/// vanishquery.SetConVars( { sv_visiblemaxplayers = n, sv_location = s,
			///     sv_max_queries_sec = n, sv_max_queries_window = n,
			///     sv_max_queries_sec_global = n } )
			LUA_FUNCTION_STATIC( SetConVars )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::TABLE );

				ConVars settings;
				settings.visible_max_players = static_cast<int32_t>(
					NumberField( LUA, "sv_visiblemaxplayers", settings.visible_max_players ) );
				settings.queries_per_second = static_cast<float>(
					NumberField( LUA, "sv_max_queries_sec", settings.queries_per_second ) );
				settings.queries_window = static_cast<float>(
					NumberField( LUA, "sv_max_queries_window", settings.queries_window ) );
				settings.queries_global = static_cast<float>(
					NumberField( LUA, "sv_max_queries_sec_global", settings.queries_global ) );

				LUA->GetField( 1, "sv_location" );
				if( LUA->IsType( -1, GarrysMod::Lua::Type::STRING ) )
					settings.location = LUA->GetString( -1 );

				LUA->Pop( 1 );

				Get( ).SetSettings( settings );
				return 0;
			}

			/// vanishquery.SetSecure( secure )
			LUA_FUNCTION_STATIC( SetSecure )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::BOOL );

				Get( ).SetSecure( LUA->GetBool( 1 ) );
				return 0;
			}

			void Initialize( GarrysMod::Lua::ILuaBase *LUA )
			{
				if( !SocketFilter::Attach( ) )
					return;

				LUA->GetField( GarrysMod::Lua::INDEX_GLOBAL, "VERSION" );
				if( LUA->IsType( -1, GarrysMod::Lua::Type::NUMBER ) )
					Replies::SetVersion( static_cast<int32_t>( LUA->GetNumber( -1 ) ) );

				LUA->Pop( 1 );

				LUA->CreateTable( );

				LUA->PushCFunction( SetEnabled );
				LUA->SetField( -2, "SetEnabled" );

				LUA->PushCFunction( SetPlayerCount );
				LUA->SetField( -2, "SetPlayerCount" );

				LUA->PushCFunction( ResetPlayers );
				LUA->SetField( -2, "ResetPlayers" );

				LUA->PushCFunction( AddPlayer );
				LUA->SetField( -2, "AddPlayer" );

				LUA->PushCFunction( SetSecure );
				LUA->SetField( -2, "SetSecure" );

				LUA->PushCFunction( SetConVars );
				LUA->SetField( -2, "SetConVars" );

				LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishquery" );
			}

			void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
			{
				Get( ).SetEnabled( false );
				SocketFilter::Detach( );

				LUA->PushNil( );
				LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishquery" );
			}
		}
	}
}
