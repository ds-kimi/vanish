#include "fields.hpp"

#include "../common/lua_args.hpp"

#include <igameevents.h>

#include <cstdint>

namespace Vanish
{
	namespace Event
	{
		using GarrysMod::Lua::ILuaBase;

		/// Writes the value on top of the stack under keyName.
		static void ApplyField( ILuaBase *LUA, IGameEvent *event, const char *keyName )
		{
			switch( LUA->GetType( -1 ) )
			{
				case GarrysMod::Lua::Type::BOOL:
					event->SetBool( keyName, LUA->GetBool( -1 ) );
					break;

				case GarrysMod::Lua::Type::NUMBER:
				{
					// event descriptors declare integer types for everything
					// the disconnect and connect events carry, so a fractional
					// value would only be truncated by the engine anyway
					event->SetInt( keyName, LuaArgs::ClampInteger( LUA->GetNumber( -1 ), INT32_MIN, INT32_MAX ) );
					break;
				}

				case GarrysMod::Lua::Type::STRING:
					event->SetString( keyName, LUA->GetString( -1 ) );
					break;

				default:
					break;
			}
		}

		void ReadFields( ILuaBase *LUA, IGameEvent *event, int iStackPos )
		{
			LUA->PushNil( );

			while( LUA->Next( iStackPos ) != 0 )
			{
				// GetString on a number key would rewrite it in place and
				// break the traversal, so non string keys are skipped untouched
				if( LUA->IsType( -2, GarrysMod::Lua::Type::STRING ) )
					ApplyField( LUA, event, LUA->GetString( -2 ) );

				LUA->Pop( 1 );
			}
		}
	}
}
