#include "delta_site.hpp"
#include "delta_promote.hpp"
#include "entity_write_info.hpp"

#include <Platform.hpp>

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86

#include <cstring>

namespace Vanish
{
	namespace DeltaHeader
	{
		//   mov edi, 1                   flags = FHDR_LEAVEPVS
		// site:
		//   mov eax, ebx                 ebx = u.m_nOldEntity
		//   mov esi, [ebp+buf]           u.m_pBuf
		//   sub eax, [ebp+base]          entity - u.m_nHeaderBase
		//   sub eax, 1
		//   js  bad_offset
		//
		// The delete path enters at site with edi = 3 and the deletion bit
		// already set. Neither moved instruction is relative.
		static const char SIGNATURE[] =
			"\xBF\x01\x00\x00\x00\x89\xD8\x8B\xB5\x2A\x2A\x2A\x2A"
			"\x2B\x85\x2A\x2A\x2A\x2A\x83\xE8\x01\x0F\x88";

		static void Emit( uint8_t *&cursor, const void *bytes, size_t size )
		{
			std::memcpy( cursor, bytes, size );
			cursor += size;
		}

		const Site &PatchSite( )
		{
			static const Site site = { SIGNATURE, sizeof( SIGNATURE ) - 1, 5, 8, 4 };
			return site;
		}

		void BuildStub( uint8_t *stub, const uint8_t *site, const uint8_t *original, int32_t info_displacement )
		{
			// pushad; pushfd; mov esi, esp; and esp, -16; sub esp, 512;
			// fxsave [esp]; sub esp, 4; push edi (flags); push ebx (entity);
			// lea eax, [ebp + disp32]
			static const uint8_t SAVE[] = {
				0x60, 0x9C, 0x89, 0xE6, 0x83, 0xE4, 0xF0, 0x81, 0xEC, 0x00, 0x02, 0x00, 0x00,
				0x0F, 0xAE, 0x04, 0x24, 0x83, 0xEC, 0x04, 0x57, 0x53, 0x8D, 0x85 };

			// push eax (info); mov eax, imm32
			static const uint8_t CALL[] = { 0x50, 0xB8 };

			// call eax; add esp, 16; fxrstor [esp]; mov esp, esi;
			// mov [esp + 4], eax (the saved edi); popfd; popad
			static const uint8_t RESTORE[] = {
				0xFF, 0xD0, 0x83, 0xC4, 0x10, 0x0F, 0xAE, 0x0C, 0x24, 0x89, 0xF4,
				0x89, 0x44, 0x24, 0x04, 0x9D, 0x61 };

			const uint32_t promote = static_cast<uint32_t>( reinterpret_cast<uintptr_t>( &Promote ) );
			const uint8_t jump = 0xE9;
			uint8_t *cursor = stub;
			Emit( cursor, SAVE, sizeof( SAVE ) );
			Emit( cursor, &info_displacement, 4 );
			Emit( cursor, CALL, sizeof( CALL ) );
			Emit( cursor, &promote, 4 );
			Emit( cursor, RESTORE, sizeof( RESTORE ) );
			Emit( cursor, original, PatchSite( ).length );
			Emit( cursor, &jump, 1 );

			const uint32_t back = static_cast<uint32_t>( reinterpret_cast<uintptr_t>( site + PatchSite( ).length ) -
				reinterpret_cast<uintptr_t>( cursor + 4 ) );
			Emit( cursor, &back, 4 );
		}

		void BuildPatch( uint8_t *patch, const uint8_t *site, const uint8_t *stub )
		{
			// jmp rel32 to the stub, then nops over the rest of the moved bytes
			const uint32_t relative = static_cast<uint32_t>(
				reinterpret_cast<uintptr_t>( stub ) - reinterpret_cast<uintptr_t>( site + 5 ) );

			patch[0] = 0xE9;
			std::memcpy( patch + 1, &relative, sizeof( relative ) );
			std::memset( patch + 5, 0x90, PatchSite( ).length - 5 );
		}
	}
}

#endif
