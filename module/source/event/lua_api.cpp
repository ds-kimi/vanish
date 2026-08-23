#include "lua_api.hpp"
#include "manager.hpp"
#include "fields.hpp"

#include <igameevents.h>

namespace Vanish
{
	namespace Event
	{
		namespace LuaAPI
		{
			/// vanishevent.Fire( name, fields ) -> boolean
			/// Fires a server game event and broadcasts it to every client,
			/// which is the only way Lua can make clients act on an event the
			/// engine never raised.
			LUA_FUNCTION_STATIC( Fire )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::STRING );
				LUA->CheckType( 2, GarrysMod::Lua::Type::TABLE );

				IGameEventManager2 *manager = Manager( );
				if( manager == nullptr )
				{
					LUA->PushBool( false );
					return 1;
				}

				// bForce, because no server side listener is registered for
				// events the game itself never fires from Lua
				IGameEvent *event = manager->CreateEvent( LUA->GetString( 1 ), true );
				if( event == nullptr )
				{
					LUA->PushBool( false );
					return 1;
				}

				ReadFields( LUA, event, 2 );

				// FireEvent takes ownership of the event either way
				LUA->PushBool( manager->FireEvent( event, false ) );
				return 1;
			}

			void Initialize( GarrysMod::Lua::ILuaBase *LUA )
			{
				LUA->CreateTable( );

				LUA->PushCFunction( Fire );
				LUA->SetField( -2, "Fire" );

				LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishevent" );
			}

			void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
			{
				LUA->PushNil( );
				LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishevent" );
			}
		}
	}
}
