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

			/// vanishquery.SetPlayerCount( count )
			LUA_FUNCTION_STATIC( SetPlayerCount )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );

				Get( ).SetVisibleCount( static_cast<int32_t>( LUA->GetNumber( 1 ) ) );
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
