#include "lua_api.hpp"
#include "hidden_set.hpp"

namespace Vanish
{
	namespace LuaAPI
	{
		/// transmitcontrol.SetHidden( entityIndex, hidden )
		LUA_FUNCTION_STATIC( SetHidden )
		{
			LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );
			LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

			const double index = LUA->GetNumber( 1 );
			if( !HiddenSet::IsEntityIndex( index ) )
				return 0;

			Hidden( ).SetHidden( static_cast<int32_t>( index ), LUA->GetBool( 2 ) );
			return 0;
		}

		/// transmitcontrol.SetExempt( playerIndex, exempt )
		LUA_FUNCTION_STATIC( SetExempt )
		{
			LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );
			LUA->CheckType( 2, GarrysMod::Lua::Type::BOOL );

			const double index = LUA->GetNumber( 1 );
			if( !HiddenSet::IsViewerIndex( index ) )
				return 0;

			Hidden( ).SetExempt( static_cast<int32_t>( index ), LUA->GetBool( 2 ) );
			return 0;
		}

		/// transmitcontrol.Reset( )
		LUA_FUNCTION_STATIC( Reset )
		{
			Hidden( ).Reset( );
			return 0;
		}

		void Initialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			LUA->CreateTable( );

			LUA->PushCFunction( SetHidden );
			LUA->SetField( -2, "SetHidden" );

			LUA->PushCFunction( SetExempt );
			LUA->SetField( -2, "SetExempt" );

			LUA->PushCFunction( Reset );
			LUA->SetField( -2, "Reset" );

			LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "transmitcontrol" );
		}

		void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			Hidden( ).Reset( );

			LUA->PushNil( );
			LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "transmitcontrol" );
		}
	}
}
