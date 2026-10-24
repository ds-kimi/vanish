#include "recv_hook_slot.hpp"

#if defined SYSTEM_WINDOWS && defined ARCHITECTURE_X86

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Vanish
{
	namespace Query
	{
		namespace RecvHookSlot
		{
			// Index of Hook_recvfrom in tier0's VCR_t. The 32-bit engine imports
			// no recvfrom: NET_ReceiveRawPacket loads this entry from g_pVCR and
			// calls it with six cdecl arguments, which is how it was verified.
			const size_t VCR_RECVFROM = 11;

			///@return the module mapping address, nullptr when none does
			static HMODULE ModuleOf( const void *address )
			{
				HMODULE module = nullptr;
				GetModuleHandleExA(
					GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
					static_cast<LPCSTR>( address ), &module );
				return module;
			}

			void **Find( )
			{
				HMODULE tier0 = GetModuleHandleA( "tier0.dll" );
				void ***vcr = tier0 != nullptr
					? reinterpret_cast<void ***>( GetProcAddress( tier0, "g_pVCR" ) )
					: nullptr;

				if( vcr == nullptr || *vcr == nullptr )
					return nullptr;

				// the stock entry is tier0's own recvfrom wrapper; anything else
				// means the table no longer has the layout this index assumes
				void **slot = *vcr + VCR_RECVFROM;
				return *slot != nullptr && ModuleOf( *slot ) == tier0 ? slot : nullptr;
			}

			RecvHook::Function Original( void *previous )
			{
				return reinterpret_cast<RecvHook::Function>( previous );
			}
		}
	}
}

#endif
