#pragma once

#include "recv_hook.hpp"

namespace Vanish
{
	namespace Query
	{
		/// The single pointer the engine reads recvfrom through, on the builds
		/// that have one: the VCR table entry or the import table slot.
		namespace RecvHookSlot
		{
			///@return the slot, nullptr when it cannot be found
			void **Find( );

			///@return the function to call for a real read, given what the slot
			///held before it was hooked
			RecvHook::Function Original( void *previous );
		}
	}
}
