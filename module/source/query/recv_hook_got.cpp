#include "recv_hook_slot.hpp"

#include "../main.hpp"
#include "../platform/elf_got.hpp"

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86_64

#include <dlfcn.h>
#include <link.h>

namespace Vanish
{
	namespace Query
	{
		namespace RecvHookSlot
		{
			void **Find( )
			{
				return ElfGot::FindJumpSlot( global::engine_loader.GetModule( ), "recvfrom" );
			}

			RecvHook::Function Original( void *previous )
			{
				link_map *engine = nullptr;
				Dl_info owner;
				if( dlinfo( global::engine_loader.GetModule( ), RTLD_DI_LINKMAP, &engine ) != 0 ||
					engine == nullptr || dladdr( previous, &owner ) == 0 )
					return nullptr;

				// Until the engine first calls recvfrom, lazy binding leaves its own
				// PLT stub in the slot. Calling that stub would make the dynamic
				// linker resolve the import and overwrite the slot, silently
				// unhooking this module, so the real function is looked up instead.
				if( reinterpret_cast<uintptr_t>( owner.dli_fbase ) == engine->l_addr )
					return reinterpret_cast<RecvHook::Function>( dlsym( RTLD_DEFAULT, "recvfrom" ) );

				return reinterpret_cast<RecvHook::Function>( previous );
			}
		}
	}
}

#endif
