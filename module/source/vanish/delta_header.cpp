#include "delta_header.hpp"
#include "hidden_set.hpp"
#include "entity_write_info.hpp"

#include <detouring/hook.hpp>
#include <scanning/symbolfinder.hpp>

#include <tier0/dbg.h>

#include <cstdint>

namespace Vanish
{
	namespace DeltaHeader
	{
		// Header flag bits, read off SV_WriteEnterPVS (4) and the leave-PVS
		// branch of CBaseServer::WriteDeltaEntities (1, or 3 once deleting).
		const int32_t FHDR_LEAVEPVS = 1;
		const int32_t FHDR_DELETE = 2;

		// SV_WriteDeltaHeader( CEntityWriteInfo *info, int entity, int flags ).
		// It has no symbol, so it is located by its prologue; the function is
		// reached from SV_WriteEnterPVS, its only distinctive caller.
		static const char SIGNATURE[] =
			"\x55\x8B\xEC\x83\xEC\x1C\x8B\x45\x08\x57\x8B\x7D\x0C\x2B\x78"
			"\x1C\x8B\x50\x24\x83\xEF\x01\x89\x55\xEC\x89\x7D\xFC\x79\x13";
		static const size_t SIGNATURE_LENGTH = sizeof( SIGNATURE ) - 1;

		typedef void ( *WriteDeltaHeader_t )( void *info, int32_t entity, int32_t flags );

		static Detouring::Hook hook;

		///@return true when this receiver must lose the entity entirely
		static bool ShouldDelete( void *info, int32_t entity, int32_t flags )
		{
			const bool leaving =
				( flags & FHDR_LEAVEPVS ) != 0 && ( flags & FHDR_DELETE ) == 0;

			if( !leaving || !Hidden( ).IsHidden( entity ) )
				return false;

			const int32_t receiver = EntityWriteInfo::Receiver( info );

			// the owner keeps their own entity, and so does anyone exempt
			return receiver != entity && !Hidden( ).IsExempt( receiver );
		}

		static void Detour( void *info, int32_t entity, int32_t flags )
		{
			if( ShouldDelete( info, entity, flags ) )
			{
				flags |= FHDR_DELETE;
				EntityWriteInfo::MarkDeleted( info, entity );
			}

			hook.GetTrampoline<WriteDeltaHeader_t>( )( info, entity, flags );
		}

		bool Attach( void *engine_module )
		{
			SymbolFinder finder;
			void *target = finder.Resolve( engine_module, SIGNATURE, SIGNATURE_LENGTH );

			if( target == nullptr )
			{
				Warning( "[vanish] SV_WriteDeltaHeader not found, hidden players stay dormant\n" );
				return false;
			}

			return hook.Create( target, reinterpret_cast<void *>( Detour ) ) && hook.Enable( );
		}

		void Detach( )
		{
			hook.Destroy( );
		}
	}
}
