#include "recv_hook_slot.hpp"

#include "../main.hpp"
#include "../platform/pe_imports.hpp"

#if defined SYSTEM_WINDOWS && defined ARCHITECTURE_X86_64

namespace Vanish
{
	namespace Query
	{
		namespace RecvHookSlot
		{
			// ws2_32 has exported recvfrom as ordinal 17 since Winsock 1.1, and
			// the 64-bit engine imports it by that ordinal rather than by name
			const uint16_t RECVFROM_ORDINAL = 17;

			void **Find( )
			{
				return PeImports::FindSlot( global::engine_loader.GetModule( ),
					"ws2_32.dll", "recvfrom", RECVFROM_ORDINAL );
			}

			RecvHook::Function Original( void *previous )
			{
				// the loader resolves imports before the module runs, so the slot
				// already holds the real function, or a hook installed before ours
				return reinterpret_cast<RecvHook::Function>( previous );
			}
		}
	}
}

#endif
