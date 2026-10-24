#include "lua_args.hpp"

namespace Vanish
{
	namespace LuaArgs
	{
		int32_t ClampInteger( double value, int32_t low, int32_t high )
		{
			// written so that NaN fails every comparison and lands on low
			if( !( value >= low ) )
				return low;

			if( !( value <= high ) )
				return high;

			return static_cast<int32_t>( value );
		}

		float ClampFloat( double value, float low, float high )
		{
			if( !( value >= low ) )
				return low;

			if( !( value <= high ) )
				return high;

			return static_cast<float>( value );
		}

		double RawNumber( GarrysMod::Lua::ILuaBase *LUA, int table, const char *key, double fallback )
		{
			LUA->PushString( key );
			LUA->RawGet( table );

			const double value = LUA->IsType( -1, GarrysMod::Lua::Type::NUMBER )
				? LUA->GetNumber( -1 ) : fallback;

			LUA->Pop( 1 );
			return value;
		}

		bool RawString( GarrysMod::Lua::ILuaBase *LUA, int table, const char *key, std::string &out )
		{
			LUA->PushString( key );
			LUA->RawGet( table );

			unsigned int length = 0;
			const char *value = LUA->IsType( -1, GarrysMod::Lua::Type::STRING )
				? LUA->GetString( -1, &length ) : nullptr;

			if( value != nullptr )
				out.assign( value, length );

			LUA->Pop( 1 );
			return value != nullptr;
		}
	}
}
