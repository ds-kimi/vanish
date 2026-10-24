#include "pe_sections.hpp"

#include <Platform.hpp>

#if defined SYSTEM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstring>

namespace Vanish
{
	namespace PeSections
	{
		std::vector<Section> Of( void *module )
		{
			std::vector<Section> sections;
			uint8_t *base = static_cast<uint8_t *>( module );
			const auto *dos = reinterpret_cast<const IMAGE_DOS_HEADER *>( base );
			if( base == nullptr || dos->e_magic != IMAGE_DOS_SIGNATURE )
				return sections;

			const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>( base + dos->e_lfanew );
			if( nt->Signature != IMAGE_NT_SIGNATURE )
				return sections;

			const IMAGE_SECTION_HEADER *header = IMAGE_FIRST_SECTION( nt );
			for( WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++header )
			{
				// discardable sections, such as .reloc, may be unmapped by now
				if( ( header->Characteristics & IMAGE_SCN_MEM_DISCARDABLE ) != 0 )
					continue;

				Section section;
				section.start = base + header->VirtualAddress;
				section.size = header->Misc.VirtualSize;
				section.writable = ( header->Characteristics & IMAGE_SCN_MEM_WRITE ) != 0;
				sections.push_back( section );
			}

			return sections;
		}

		std::vector<uintptr_t *> FindWord( const std::vector<Section> &sections, uintptr_t value, bool writable_only )
		{
			std::vector<uintptr_t *> found;
			for( const Section &section : sections )
			{
				if( writable_only && !section.writable )
					continue;

				uintptr_t *word = reinterpret_cast<uintptr_t *>( section.start );
				uintptr_t *end = word + section.size / sizeof( uintptr_t );
				for( ; word < end; ++word )
					if( *word == value )
						found.push_back( word );
			}

			return found;
		}

		const uint8_t *FindBytes( const std::vector<Section> &sections, const void *bytes, size_t size )
		{
			for( const Section &section : sections )
			{
				if( section.size < size )
					continue;

				const uint8_t *end = section.start + section.size - size;
				for( const uint8_t *at = section.start; at <= end; ++at )
					if( std::memcmp( at, bytes, size ) == 0 )
						return at;
			}

			return nullptr;
		}
	}
}

#endif
