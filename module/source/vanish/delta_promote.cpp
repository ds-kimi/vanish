#include "delta_promote.hpp"
#include "entity_write_info.hpp"
#include "hidden_set.hpp"

namespace Vanish
{
	namespace DeltaHeader
	{
		// Header flag bits, read off SV_WriteEnterPVS (4) and the leave-PVS
		// branch of CBaseServer::WriteDeltaEntities (1, or 3 once deleting).
		const int32_t FHDR_LEAVEPVS = 1;
		const int32_t FHDR_DELETE = 2;

		///@return true when this receiver must lose the entity entirely
		static bool ShouldDelete( const void *info, int32_t entity, int32_t flags )
		{
			const bool leaving =
				( flags & FHDR_LEAVEPVS ) != 0 && ( flags & FHDR_DELETE ) == 0;

			if( !leaving || !Hidden( ).IsHidden( entity ) )
				return false;

			const int32_t receiver = EntityWriteInfo::Receiver( info );

			// the owner keeps their own entity, and so does anyone exempt
			return receiver != entity && !Hidden( ).IsExempt( receiver );
		}

		int32_t Promote( void *info, int32_t entity, int32_t flags )
		{
			if( info == nullptr || !ShouldDelete( info, entity, flags ) )
				return flags;

			EntityWriteInfo::MarkDeleted( info, entity );
			return flags | FHDR_DELETE;
		}
	}
}
