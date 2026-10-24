#include "recv_hook.hpp"
#include "recv_hook_slot.hpp"

#include "../platform/memory.hpp"

#if !( defined SYSTEM_LINUX && defined ARCHITECTURE_X86 )

namespace Vanish
{
	namespace Query
	{
		namespace RecvHook
		{
			static void **slot = nullptr;
			static void *previous = nullptr;
			static void *installed = nullptr;

			Function Install( Function replacement )
			{
				if( slot != nullptr )
					return nullptr;

				void **target = RecvHookSlot::Find( );
				if( target == nullptr || *target == nullptr )
					return nullptr;

				void *before = *target;
				Function original = RecvHookSlot::Original( before );
				if( original == nullptr ||
					!Memory::WritePointer( target, reinterpret_cast<void *>( replacement ) ) )
					return nullptr;

				slot = target;
				previous = before;
				installed = reinterpret_cast<void *>( replacement );
				return original;
			}

			bool Remove( )
			{
				if( slot == nullptr )
					return true;

				// another hook chained onto this one: writing the old pointer back
				// would cut it out, and unloading would leave it calling freed code
				if( *slot != installed )
					return false;

				if( !Memory::WritePointer( slot, previous ) )
					return false;

				slot = nullptr;
				return true;
			}
		}
	}
}

#endif
