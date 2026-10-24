#include "lua_api.hpp"
#include "roster.hpp"
#include "state.hpp"

#include "../common/lua_args.hpp"

namespace Vanish
{
	namespace Query
	{
		namespace LuaAPI
		{
			/// vanishquery.SetPlayerCount( count, bots )
			LUA_FUNCTION_STATIC( SetPlayerCount )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );
				LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );

				const int32_t limit = static_cast<int32_t>( MAX_PLAYERS );
				Roster::SetCounts(
					LuaArgs::ClampInteger( LUA->GetNumber( 1 ), 0, limit ),
					LuaArgs::ClampInteger( LUA->GetNumber( 2 ), 0, limit ) );

				return 0;
			}

			/// vanishquery.ResetPlayers( )
			LUA_FUNCTION_STATIC( ResetPlayers )
			{
				Roster::Clear( );
				return 0;
			}

			/// vanishquery.AddPlayer( name, score, seconds )
			LUA_FUNCTION_STATIC( AddPlayer )
			{
				// every check comes before anything with a destructor exists:
				// a failed check raises a Lua error, which skips destructors on
				// builds where LuaJIT unwinds with longjmp
				LUA->CheckType( 1, GarrysMod::Lua::Type::STRING );
				LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
				LUA->CheckType( 3, GarrysMod::Lua::Type::NUMBER );

				unsigned int length = 0;
				const char *name = LUA->GetString( 1, &length );
				Roster::Add( name, length,
					LuaArgs::ClampInteger( LUA->GetNumber( 2 ), INT32_MIN, INT32_MAX ),
					LuaArgs::ClampFloat( LUA->GetNumber( 3 ), 0.0f, 1e9f ) );

				return 0;
			}

			void RegisterRoster( GarrysMod::Lua::ILuaBase *LUA )
			{
				LUA->PushCFunction( SetPlayerCount );
				LUA->SetField( -2, "SetPlayerCount" );

				LUA->PushCFunction( ResetPlayers );
				LUA->SetField( -2, "ResetPlayers" );

				LUA->PushCFunction( AddPlayer );
				LUA->SetField( -2, "AddPlayer" );
			}
		}
	}
}
