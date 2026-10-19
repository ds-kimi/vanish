#include "delta_header.hpp"
#include "hidden_set.hpp"
#include "entity_write_info.hpp"

#include <Platform.hpp>
#include <scanning/symbolfinder.hpp>

#include <tier0/dbg.h>

#include <cstdint>
#include <cstring>

#if defined SYSTEM_WINDOWS
#include <detouring/hook.hpp>
#else
#include "../platform/code_patch.hpp"
#endif

namespace Vanish
{
	namespace DeltaHeader
	{
		// Header flag bits, read off SV_WriteEnterPVS (4) and the leave-PVS
		// branch of CBaseServer::WriteDeltaEntities (1, or 3 once deleting).
		const int32_t FHDR_LEAVEPVS = 1;
		const int32_t FHDR_DELETE = 2;

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

		///@return flags with the delete bit added when the header must delete
		static int32_t Promote( void *info, int32_t entity, int32_t flags )
		{
			if( !ShouldDelete( info, entity, flags ) )
				return flags;

			EntityWriteInfo::MarkDeleted( info, entity );
			return flags | FHDR_DELETE;
		}

#if defined SYSTEM_WINDOWS

		// SV_WriteDeltaHeader( CEntityWriteInfo *info, int entity, int flags ).
		// It has no symbol, so it is located by its prologue; the function is
		// reached from SV_WriteEnterPVS, its only distinctive caller.
		static const char SIGNATURE[] =
			"\x55\x8B\xEC\x83\xEC\x1C\x8B\x45\x08\x57\x8B\x7D\x0C\x2B\x78"
			"\x1C\x8B\x50\x24\x83\xEF\x01\x89\x55\xEC\x89\x7D\xFC\x79\x13";
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

#else

		// The Linux engine inlines SV_WriteDeltaHeader into every caller, so
		// there is no function to detour. The leave-PVS branch of
		// CBaseServer::WriteDeltaEntities is patched instead, at the point
		// where both of its paths have settled the flags:
		//
		//   mov edi, 1                   flags = FHDR_LEAVEPVS
		// site:
		//   mov eax, ebx                 ebx = u.m_nOldEntity
		//   mov esi, [ebp+buf]           u.m_pBuf
		//   sub eax, [ebp+base]          entity - u.m_nHeaderBase
		//   sub eax, 1
		//   js  bad_offset
		//
		// The delete path enters at site with edi = 3 and the deletion bit
		// already set. The eight bytes at site are moved into a stub that
		// calls Promote and writes its result back into edi.
		static const char SIGNATURE[] =
			"\xBF\x01\x00\x00\x00\x89\xD8\x8B\xB5\x2A\x2A\x2A\x2A"
			"\x2B\x85\x2A\x2A\x2A\x2A\x83\xE8\x01\x0F\x88";
		static const size_t SIGNATURE_LENGTH = sizeof( SIGNATURE ) - 1;

		const size_t SITE_OFFSET = 5;
		const size_t SITE_LENGTH = 8;

		// where the m_pBuf displacement sits within the patched bytes
		const size_t BUFFER_DISPLACEMENT = 4;

		// m_pBuf's offset in CEntityWriteInfo, to get from it to the struct
		const int32_t OFFSET_BUFFER = 0x24;

		const size_t STUB_SIZE = 64;

		static uint8_t *site = nullptr;
		static uint8_t original[SITE_LENGTH];
		static uint8_t *stub = nullptr;

		/// Emits bytes into the stub, advancing the cursor.
		static void Emit( uint8_t *&cursor, const void *bytes, size_t size )
		{
			std::memcpy( cursor, bytes, size );
			cursor += size;
		}

		static void EmitWord( uint8_t *&cursor, uint32_t word )
		{
			Emit( cursor, &word, sizeof( word ) );
		}

		/// Builds the stub the patched site jumps to. The engine's registers
		/// survive it untouched apart from edi, which carries the new flags.
		static void BuildStub( uint8_t *cursor, int32_t info_displacement )
		{
			static const uint8_t SAVE[] = {
				0x60,                   // pushad
				0x9C,                   // pushfd
				0x89, 0xE6,             // mov esi, esp
				0x83, 0xE4, 0xF0,       // and esp, -16
				0x83, 0xEC, 0x04,       // sub esp, 4, aligned again after 3 pushes
				0x57,                   // push edi        flags
				0x53,                   // push ebx        entity
				0x8D, 0x85              // lea eax, [ebp + displacement]
			};

			static const uint8_t CALL[] = {
				0x50,                   // push eax        info
				0xB8                    // mov eax, Promote
			};

			static const uint8_t RESTORE[] = {
				0xFF, 0xD0,             // call eax
				0x89, 0xF4,             // mov esp, esi
				0x89, 0x44, 0x24, 0x04, // mov [esp + 4], eax   saved edi
				0x9D,                   // popfd
				0x61                    // popad
			};

			Emit( cursor, SAVE, sizeof( SAVE ) );
			EmitWord( cursor, static_cast<uint32_t>( info_displacement ) );
			Emit( cursor, CALL, sizeof( CALL ) );
			EmitWord( cursor, static_cast<uint32_t>(
				reinterpret_cast<uintptr_t>( Promote ) ) );
			Emit( cursor, RESTORE, sizeof( RESTORE ) );

			// the instructions the jump displaced, neither of them relative
			Emit( cursor, original, SITE_LENGTH );

			const uint8_t jump = 0xE9;
			Emit( cursor, &jump, 1 );
			EmitWord( cursor, static_cast<uint32_t>(
				reinterpret_cast<uintptr_t>( site + SITE_LENGTH ) -
				reinterpret_cast<uintptr_t>( cursor + 4 ) ) );
		}

		bool Attach( void *engine_module )
		{
			SymbolFinder finder;
			uint8_t *match = static_cast<uint8_t *>(
				finder.Resolve( engine_module, SIGNATURE, SIGNATURE_LENGTH ) );

			if( match == nullptr )
			{
				Warning( "[vanish] WriteDeltaEntities leave-PVS branch not found, hidden players stay dormant\n" );
				return false;
			}

			stub = static_cast<uint8_t *>( CodePatch::Allocate( STUB_SIZE ) );
			if( stub == nullptr )
				return false;

			site = match + SITE_OFFSET;
			std::memcpy( original, site, SITE_LENGTH );

			int32_t buffer_displacement = 0;
			std::memcpy( &buffer_displacement, site + BUFFER_DISPLACEMENT,
				sizeof( buffer_displacement ) );

			BuildStub( stub, buffer_displacement - OFFSET_BUFFER );

			uint8_t patch[SITE_LENGTH] = { 0xE9 };
			const uint32_t relative = static_cast<uint32_t>(
				reinterpret_cast<uintptr_t>( stub ) -
				reinterpret_cast<uintptr_t>( site + 5 ) );
			std::memcpy( patch + 1, &relative, sizeof( relative ) );
			std::memset( patch + 5, 0x90, SITE_LENGTH - 5 );

			if( !CodePatch::Write( site, patch, SITE_LENGTH ) )
			{
				CodePatch::Free( stub, STUB_SIZE );
				stub = nullptr;
				site = nullptr;
				return false;
			}

			return true;
		}

		void Detach( )
		{
			if( site == nullptr )
				return;

			CodePatch::Write( site, original, SITE_LENGTH );
			CodePatch::Free( stub, STUB_SIZE );

			site = nullptr;
			stub = nullptr;
		}

#endif
	}
}
