#include "delta_header.hpp"
#include "delta_promote.hpp"

#include <Platform.hpp>

#if defined SYSTEM_WINDOWS

#include <scanning/symbolfinder.hpp>
#include <detouring/hook.hpp>

#include <tier0/dbg.h>

#include <cstdint>

namespace Vanish
{
	namespace DeltaHeader
	{
		// SV_WriteDeltaHeader( CEntityWriteInfo &info, int entity, int flags ).
		// It has no symbol, so it is located by its prologue. Every leave-PVS
		// header goes through it on both Windows engines; only the enter-PVS
		// path inlines its own copy, which never carries the leave flag.
#if defined ARCHITECTURE_X86_64
		static const char SIGNATURE[] =
			"\x48\x89\x5C\x24\x08\x48\x89\x6C\x24\x10\x48\x89\x74\x24\x18\x57"
			"\x48\x83\xEC\x20\x48\x8B\x59\x38\x8B\xEA\x2B\x51\x2C\x41\x8B\xF0"
			"\x83\xEA\x01\x48\x8B\xF9\x79";
#else
		static const char SIGNATURE[] =
			"\x55\x8B\xEC\x83\xEC\x1C\x8B\x45\x08\x57\x8B\x7D\x0C\x2B\x78"
			"\x1C\x8B\x50\x24\x83\xEF\x01\x89\x55\xEC\x89\x7D\xFC\x79\x13";
#endif
		static const size_t SIGNATURE_LENGTH = sizeof( SIGNATURE ) - 1;

		typedef void ( *WriteDeltaHeader_t )( void *info, int32_t entity, int32_t flags );

		static Detouring::Hook hook;

		static void Detour( void *info, int32_t entity, int32_t flags )
		{
			hook.GetTrampoline<WriteDeltaHeader_t>( )( info, entity, Promote( info, entity, flags ) );
		}

		bool Attach( void *engine_module )
		{
			SymbolFinder finder;
			void *target = engine_module != nullptr
				? finder.Resolve( engine_module, SIGNATURE, SIGNATURE_LENGTH )
				: nullptr;

			if( target == nullptr )
			{
				Warning( "[vanish] SV_WriteDeltaHeader not found, hidden players stay dormant\n" );
				return false;
			}

			if( hook.Create( target, reinterpret_cast<void *>( Detour ) ) && hook.Enable( ) )
				return true;

			hook.Destroy( );
			return false;
		}

		void Detach( )
		{
			hook.Destroy( );
		}
	}
}

#endif
