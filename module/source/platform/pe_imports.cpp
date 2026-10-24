#include "pe_imports.hpp"

#include <Platform.hpp>

#if defined SYSTEM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstring>

namespace Vanish
{
	namespace PeImports
	{
		///@return the module's import descriptors, nullptr when it has none
		static const IMAGE_IMPORT_DESCRIPTOR *Descriptors( const uint8_t *base )
		{
			const auto *dos = reinterpret_cast<const IMAGE_DOS_HEADER *>( base );
			if( dos->e_magic != IMAGE_DOS_SIGNATURE )
				return nullptr;

			const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>( base + dos->e_lfanew );
			if( nt->Signature != IMAGE_NT_SIGNATURE )
				return nullptr;

			const IMAGE_DATA_DIRECTORY &imports = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
			return imports.VirtualAddress != 0 && imports.Size != 0
				? reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR *>( base + imports.VirtualAddress )
				: nullptr;
		}

		///@return whether a lookup table entry names function or its ordinal
		static bool Matches( const uint8_t *base, const IMAGE_THUNK_DATA &entry, const char *function, uint16_t ordinal )
		{
			if( IMAGE_SNAP_BY_ORDINAL( entry.u1.Ordinal ) )
				return IMAGE_ORDINAL( entry.u1.Ordinal ) == ordinal;

			const auto *named = reinterpret_cast<const IMAGE_IMPORT_BY_NAME *>( base + entry.u1.AddressOfData );
			return std::strcmp( reinterpret_cast<const char *>( named->Name ), function ) == 0;
		}

		/// Walks one DLL's lookup table beside its address table.
		static void **FindInDescriptor( uint8_t *base, const IMAGE_IMPORT_DESCRIPTOR &descriptor,
			const char *function, uint16_t ordinal )
		{
			// a module bound without a lookup table only has the address table
			// left, whose entries are already resolved and cannot be matched
			if( descriptor.OriginalFirstThunk == 0 )
				return nullptr;

			const auto *lookup = reinterpret_cast<const IMAGE_THUNK_DATA *>( base + descriptor.OriginalFirstThunk );
			auto *addresses = reinterpret_cast<IMAGE_THUNK_DATA *>( base + descriptor.FirstThunk );
			for( ; lookup->u1.AddressOfData != 0; ++lookup, ++addresses )
				if( Matches( base, *lookup, function, ordinal ) )
					return reinterpret_cast<void **>( &addresses->u1.Function );

			return nullptr;
		}

		void **FindSlot( void *module, const char *library, const char *function, uint16_t ordinal )
		{
			uint8_t *base = static_cast<uint8_t *>( module );
			const IMAGE_IMPORT_DESCRIPTOR *descriptor = base != nullptr ? Descriptors( base ) : nullptr;
			if( descriptor == nullptr )
				return nullptr;

			for( ; descriptor->Name != 0; ++descriptor )
			{
				const char *name = reinterpret_cast<const char *>( base + descriptor->Name );
				if( _stricmp( name, library ) != 0 )
					continue;

				void **slot = FindInDescriptor( base, *descriptor, function, ordinal );
				if( slot != nullptr )
					return slot;
			}

			return nullptr;
		}
	}
}

#endif
