#include "delta_site.hpp"
#include "delta_promote.hpp"

#include <Platform.hpp>

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86_64

#include <cstring>

namespace Vanish
{
	namespace DeltaHeader
	{
		//   mov r13d, 1                  flags = FHDR_LEAVEPVS
		// site:
		//   mov esi, ebx                 ebx = u.m_nOldEntity
		//   mov r12, [rbp+buf]           u.m_pBuf
		//   sub esi, [rbp+base]          entity - u.m_nHeaderBase
		//   sub esi, 1
		//   js  bad_offset
		//
		// The delete path enters at site with r13d = 3 and the deletion bit
		// already set. None of the 15 moved bytes is RIP relative, and both
		// jumps into the branch land on site itself, never inside it.
		static const char SIGNATURE[] =
			"\x41\xBD\x01\x00\x00\x00\x89\xDE\x4C\x8B\xA5\x2A\x2A\x2A\x2A"
			"\x2B\xB5\x2A\x2A\x2A\x2A\x83\xEE\x01\x0F\x88";

		static void Emit( uint8_t *&cursor, const void *bytes, size_t size )
		{
			std::memcpy( cursor, bytes, size );
			cursor += size;
		}

		const Site &PatchSite( )
		{
			static const Site site = { SIGNATURE, sizeof( SIGNATURE ) - 1, 6, 15, 5 };
			return site;
		}

		// lea rsp, [rsp - 128] past the red zone; pushfq; push rax, rcx,
		// rdx, rsi, rdi, r8, r9, r10, r11, rbx; mov esi, ebx (entity);
		// mov edx, r13d (flags); lea rdi, [rbp + disp32]
		static const uint8_t SAVE[] = {
			0x48, 0x8D, 0x64, 0x24, 0x80, 0x9C, 0x50, 0x51, 0x52, 0x56, 0x57,
			0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53, 0x53,
			0x89, 0xDE, 0x44, 0x89, 0xEA, 0x48, 0x8D, 0xBD };

		// mov rbx, rsp; and rsp, -16; sub rsp, 512; fxsave64 [rsp];
		// mov rax, imm64
		static const uint8_t CALL[] = {
			0x48, 0x89, 0xE3, 0x48, 0x83, 0xE4, 0xF0, 0x48, 0x81, 0xEC, 0x00, 0x02, 0x00, 0x00,
			0x48, 0x0F, 0xAE, 0x04, 0x24, 0x48, 0xB8 };

		// call rax; fxrstor64 [rsp]; mov rsp, rbx; mov r13d, eax; pop rbx,
		// r11, r10, r9, r8, rdi, rsi, rdx, rcx, rax; popfq; lea rsp, [rsp + 128]
		static const uint8_t RESTORE[] = {
			0xFF, 0xD0, 0x48, 0x0F, 0xAE, 0x0C, 0x24, 0x48, 0x89, 0xDC, 0x41, 0x89, 0xC5,
			0x5B, 0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58, 0x5F, 0x5E, 0x5A, 0x59, 0x58,
			0x9D, 0x48, 0x8D, 0xA4, 0x24, 0x80, 0x00, 0x00, 0x00 };

		// jmp qword ptr [rip + 0], followed by the absolute target
		static const uint8_t JUMP[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };

		void BuildStub( uint8_t *stub, const uint8_t *site, const uint8_t *original, int32_t info_displacement )
		{
			const uint64_t promote = reinterpret_cast<uintptr_t>( &Promote );
			const uint64_t back = reinterpret_cast<uintptr_t>( site + PatchSite( ).length );
			uint8_t *cursor = stub;
			Emit( cursor, SAVE, sizeof( SAVE ) );
			Emit( cursor, &info_displacement, 4 );
			Emit( cursor, CALL, sizeof( CALL ) );
			Emit( cursor, &promote, 8 );
			Emit( cursor, RESTORE, sizeof( RESTORE ) );
			Emit( cursor, original, PatchSite( ).length );
			Emit( cursor, JUMP, sizeof( JUMP ) );
			Emit( cursor, &back, 8 );
		}

		void BuildPatch( uint8_t *patch, const uint8_t *site, const uint8_t *stub )
		{
			// an absolute jump, since the stub can be mapped anywhere relative
			// to the engine, then nops over the rest of the moved bytes
			const uint64_t target = reinterpret_cast<uintptr_t>( stub );

			( void )site;
			std::memcpy( patch, JUMP, sizeof( JUMP ) );
			std::memcpy( patch + sizeof( JUMP ), &target, sizeof( target ) );
			std::memset( patch + 14, 0x90, PatchSite( ).length - 14 );
		}
	}
}

#endif
