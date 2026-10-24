#include "lua_api.hpp"
#include "table.hpp"

namespace Vanish
{
	namespace UserInfo
	{
		namespace LuaAPI
		{
			/// vanishuserinfo.SetHidden( entityIndex, hidden ) -> boolean
			LUA_FUNCTION_STATIC( SetHidden )
			{
				LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );
				LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

				// written so that NaN fails the test instead of reaching the cast
				const double index = LUA->GetNumber( 1 );
				if( !( index >= 1 && index <= 128 ) )
				{
					LUA->PushBool( false );
					return 1;
				}

				// the table is indexed by client slot, which is the entity
				// index without the world entity
				const int32_t slot = static_cast<int32_t>( index ) - 1;

				LUA->PushBool( LUA->GetBool( 2 ) ? Hide( slot ) : Show( slot ) );
				return 1;
			}

			/// vanishuserinfo.Reset( )
			LUA_FUNCTION_STATIC( ResetEntries )
			{
				Reset( );
				return 0;
			}

			void Initialize( GarrysMod::Lua::ILuaBase *LUA )
			{
				LUA->CreateTable( );

				LUA->PushCFunction( SetHidden );
				LUA->SetField( -2, "SetHidden" );

				LUA->PushCFunction( ResetEntries );
				LUA->SetField( -2, "Reset" );

				LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishuserinfo" );
			}

			void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
			{
				// leaving an entry blank after unload would strand the player
				// nameless on every client until the map changes
				Reset( );

				LUA->PushNil( );
				LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "vanishuserinfo" );
			}
		}
	}
}
