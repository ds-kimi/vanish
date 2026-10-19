#include "game_server.hpp"

#include "../main.hpp"

#include <GarrysMod/InterfacePointers.hpp>
#include <Platform.hpp>

#if defined SYSTEM_LINUX
#include "../platform/elf_module.hpp"
#endif

namespace Vanish
{
	namespace GameServer
	{
		IServer *Get( )
		{
			static IServer *server = nullptr;
			if( server != nullptr )
				return server;

#if defined SYSTEM_LINUX

			// CGameServer derives from IServer first, so the object and its
			// IServer base share an address
			server = static_cast<IServer *>( ElfModule::FindInstance(
				global::engine_loader.GetModule( ), "11CGameServer" ) );

#else

			server = InterfacePointers::Server( );

#endif

			return server;
		}
	}
}
