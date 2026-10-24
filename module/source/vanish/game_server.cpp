#include "game_server.hpp"

#include "../main.hpp"

#include <GarrysMod/InterfacePointers.hpp>
#include <Platform.hpp>

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86
#include "../platform/elf_instance.hpp"
#elif defined SYSTEM_WINDOWS && defined ARCHITECTURE_X86
#include "../platform/pe_instance.hpp"
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

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86

			// The 32-bit engine is stripped of its sv symbol. CGameServer derives
			// from IServer first, so the object and its IServer base share an
			// address
			server = static_cast<IServer *>( ElfInstance::Find(
				global::engine_loader.GetModule( ), "11CGameServer" ) );

#elif defined SYSTEM_WINDOWS && defined ARCHITECTURE_X86

			// The 32-bit Windows engine exports no sv either, and the signature
			// garrysmod_common falls back on no longer matches it. MSVC RTTI
			// leads to the same single CGameServer
			server = static_cast<IServer *>( PeInstance::Find(
				global::engine_loader.GetModule( ), ".?AVCGameServer@@" ) );

#else

			// sv is exported by name from both 64-bit engines
			server = InterfacePointers::Server( );

#endif

			return server;
		}
	}
}
