#include "lua_api.hpp"
#include "replies.hpp"
#include "socket_filter.hpp"
#include "state.hpp"

#include "../common/lua_args.hpp"

#include <cstdint>

namespace Vanish
{
	namespace Query
	{
		namespace LuaAPI
		{
			void Initialize( GarrysMod::Lua::ILuaBase *LUA )
			{
				LUA->GetField( GarrysMod::Lua::INDEX_GLOBAL, "VERSION" );
				const int32_t version = LUA->IsType( -1, GarrysMod::Lua::Type::NUMBER )
					? LuaArgs::ClampInteger( LUA->GetNumber( -1 ), 0, INT32_MAX ) : 0;
				LUA->Pop( 1 );

				Replies::SetVersion( version );

				if( !SocketFilter::Attach( ) )
					return;

				LUA->CreateTable( );
				RegisterRoster( LUA );
				RegisterSettings( LUA );
				LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishquery" );
			}

			void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
			{
				SocketFilter::Detach( );

				LUA->PushNil( );
				LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishquery" );
			}
		}
	}
}
