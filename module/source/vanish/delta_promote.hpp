#pragma once

#include <cstdint>

namespace Vanish
{
	namespace DeltaHeader
	{
		/// Decides the header flags one receiver gets for one entity, marking
		/// the entity deleted for that receiver when the flags are promoted.
		///@param info the engine's CEntityWriteInfo for this receiver
		///@return flags with the delete bit added when the header must delete
		int32_t Promote( void *info, int32_t entity, int32_t flags );
	}
}
