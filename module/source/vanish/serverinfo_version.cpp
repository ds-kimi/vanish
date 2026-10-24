#include "serverinfo.hpp"
#include "build_number.hpp"

#include <GarrysMod/InterfacePointers.hpp>

#include <eiface.h>

namespace Vanish
{
	namespace ServerInfo
	{
		/// Pushes the engine build number the status line prints beside the patch
		/// version. A dedicated server's steam.inf carries only PatchVersion, so
		/// this is the only place the real number can come from.
		LUA_FUNCTION_STATIC( GetServerVersion )
		{
			IVEngineServer *engine = InterfacePointers::VEngineServer( );
			int version = engine != nullptr ? engine->GetServerVersion( ) : 0;

			if( version <= 0 )
				version = BuildNumber::Get( );

			if( version <= 0 )
			{
				LUA->PushNil( );
				return 1;
			}

			LUA->PushNumber( version );
			return 1;
		}

		void RegisterVersion( GarrysMod::Lua::ILuaBase *LUA )
		{
			LUA->PushCFunction( GetServerVersion );
			LUA->SetField( -2, "GetServerVersion" );
		}
	}
}
