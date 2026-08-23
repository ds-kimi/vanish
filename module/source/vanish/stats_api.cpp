#include "stats_api.hpp"
#include "stats.hpp"

namespace Vanish
{
	namespace Stats
	{
		/// vanishinfo.GetNetStats( ) -> inKBs, outKBs
		LUA_FUNCTION_STATIC( GetNetStats )
		{
			float in = 0.0f, out = 0.0f;
			if( !NetStats( in, out ) )
			{
				LUA->PushNil( );
				return 1;
			}

			LUA->PushNumber( in );
			LUA->PushNumber( out );
			return 2;
		}

		/// vanishinfo.GetSpawnCount( ) -> number|nil
		LUA_FUNCTION_STATIC( GetSpawnCount )
		{
			const int count = SpawnCount( );
			if( count < 0 )
			{
				LUA->PushNil( );
				return 1;
			}

			LUA->PushNumber( count );
			return 1;
		}

		/// vanishinfo.GetCPUUsage( ) -> number percentage of one core
		LUA_FUNCTION_STATIC( GetCPUUsage )
		{
			LUA->PushNumber( CPUUsage( ) );
			return 1;
		}

		void Register( GarrysMod::Lua::ILuaBase *LUA )
		{
			LUA->PushCFunction( GetNetStats );
			LUA->SetField( -2, "GetNetStats" );

			LUA->PushCFunction( GetSpawnCount );
			LUA->SetField( -2, "GetSpawnCount" );

			LUA->PushCFunction( GetCPUUsage );
			LUA->SetField( -2, "GetCPUUsage" );
		}
	}
}
