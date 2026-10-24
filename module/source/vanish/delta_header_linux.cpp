#include "delta_header.hpp"
#include "delta_site.hpp"
#include "entity_write_info.hpp"

#include "../platform/code_patch.hpp"
#include "../platform/memory.hpp"

#include <Platform.hpp>

#if defined SYSTEM_LINUX

#include <scanning/symbolfinder.hpp>

#include <tier0/dbg.h>

#include <cstring>

namespace Vanish
{
	namespace DeltaHeader
	{
		static uint8_t *site = nullptr;
		static uint8_t original[MAX_SITE_LENGTH];
		static uint8_t *stub = nullptr;

		///@return the first byte to patch, nullptr when the branch is not found
		static uint8_t *FindSite( void *engine_module )
		{
			const Site &where = PatchSite( );
			SymbolFinder finder;
			uint8_t *match = engine_module != nullptr ? static_cast<uint8_t *>(
				finder.Resolve( engine_module, where.signature, where.signature_length ) ) : nullptr;

			return match != nullptr ? match + where.offset : nullptr;
		}

		bool Attach( void *engine_module )
		{
			const Site &where = PatchSite( );
			uint8_t *target = FindSite( engine_module );
			if( target == nullptr )
			{
				Warning( "[vanish] WriteDeltaEntities leave-PVS branch not found, hidden players stay dormant\n" );
				return false;
			}

			uint8_t *block = static_cast<uint8_t *>( CodePatch::Allocate( STUB_SIZE ) );
			if( block == nullptr )
				return false;

			int32_t buffer_displacement = 0;
			std::memcpy( original, target, where.length );
			std::memcpy( &buffer_displacement, target + where.buffer_operand, sizeof( buffer_displacement ) );
			BuildStub( block, target, original,
				buffer_displacement - static_cast<int32_t>( EntityWriteInfo::OFFSET_BUFFER ) );

			uint8_t patch[MAX_SITE_LENGTH];
			BuildPatch( patch, target, block );
			if( !CodePatch::Seal( block, STUB_SIZE ) || !CodePatch::Write( target, patch, where.length ) )
			{
				CodePatch::Free( block, STUB_SIZE );
				return false;
			}

			site = target;
			stub = block;
			return true;
		}

		void Detach( )
		{
			if( site == nullptr )
				return;

			// the engine keeps jumping into the stub when the site cannot be
			// restored, and the stub calls into this module, so both stay
			if( !CodePatch::Write( site, original, PatchSite( ).length ) )
			{
				Warning( "[vanish] could not restore WriteDeltaEntities, keeping the module loaded\n" );
				Memory::PinModule( );
				site = nullptr;
				return;
			}

			CodePatch::Free( stub, STUB_SIZE );
			site = nullptr;
			stub = nullptr;
		}
	}
}

#endif
