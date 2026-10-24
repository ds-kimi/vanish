#include "recv_hook.hpp"

#include "../main.hpp"
#include "../platform/elf_calls.hpp"

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86

#include <vector>

namespace Vanish
{
	namespace Query
	{
		namespace RecvHook
		{
			// The 32-bit engine has neither a VCR table nor a PLT for recvfrom:
			// it calls libc directly, so its own call sites are rewritten. libc's
			// recvfrom itself must not be detoured, it opens with a call to a PC
			// thunk that does not survive being moved.
			static std::vector<ElfCalls::CallSite> call_sites;

			Function Install( Function replacement )
			{
				if( !call_sites.empty( ) )
					return nullptr;

				return reinterpret_cast<Function>( ElfCalls::Redirect(
					global::engine_loader.GetModule( ), "recvfrom",
					reinterpret_cast<void *>( replacement ), call_sites ) );
			}

			bool Remove( )
			{
				// call sites are private to the engine, nothing can chain onto them
				ElfCalls::Restore( call_sites );
				return true;
			}
		}
	}
}

#endif
