#include "main.hpp"

#include "vanish/hooks.hpp"
#include "vanish/lua_api.hpp"
#include "vanish/serverinfo.hpp"
#include "query/lua_api.hpp"
#include "event/lua_api.hpp"

namespace global
{
	// one loader for the module: the hooks resolve engine functions against
	// this handle instead of opening the library again
	const SourceSDK::FactoryLoader engine_loader( "engine" );
}

GMOD_MODULE_OPEN( )
{
	Vanish::Hooks::Attach( );
	Vanish::LuaAPI::Initialize( LUA );
	Vanish::ServerInfo::Initialize( LUA );
	Vanish::Query::LuaAPI::Initialize( LUA );
	Vanish::Event::LuaAPI::Initialize( LUA );
	return 0;
}

GMOD_MODULE_CLOSE( )
{
	Vanish::Hooks::Detach( );
	Vanish::LuaAPI::Deinitialize( LUA );
	Vanish::ServerInfo::Deinitialize( LUA );
	Vanish::Query::LuaAPI::Deinitialize( LUA );
	Vanish::Event::LuaAPI::Deinitialize( LUA );
	return 0;
}
