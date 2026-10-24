#include "elf_got.hpp"

#include <Platform.hpp>

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86_64

#include <cstring>

#include <dlfcn.h>
#include <elf.h>
#include <link.h>

namespace Vanish
{
	namespace ElfGot
	{
		///@return the value of a dynamic section entry, 0 when absent
		static ElfW( Xword ) DynamicValue( const link_map *map, ElfW( Sxword ) tag )
		{
			for( const ElfW( Dyn ) *entry = map->l_ld; entry->d_tag != DT_NULL; ++entry )
				if( entry->d_tag == tag )
					return entry->d_un.d_val;

			return 0;
		}

		///@return a dynamic section address as a pointer into the loaded image
		static uintptr_t DynamicAddress( const link_map *map, ElfW( Sxword ) tag )
		{
			const uintptr_t value = DynamicValue( map, tag );
			if( value == 0 )
				return 0;

			// glibc rebases these entries in place when it loads the module,
			// other loaders leave them as offsets from the load base
			return value < map->l_addr ? map->l_addr + value : value;
		}

		void **FindJumpSlot( void *handle, const char *symbol )
		{
			link_map *map = nullptr;
			if( handle == nullptr || dlinfo( handle, RTLD_DI_LINKMAP, &map ) != 0 || map == nullptr )
				return nullptr;

			const auto *relocations = reinterpret_cast<const ElfW( Rela ) *>( DynamicAddress( map, DT_JMPREL ) );
			const auto *symbols = reinterpret_cast<const ElfW( Sym ) *>( DynamicAddress( map, DT_SYMTAB ) );
			const char *strings = reinterpret_cast<const char *>( DynamicAddress( map, DT_STRTAB ) );
			const ElfW( Xword ) size = DynamicValue( map, DT_PLTRELSZ );
			const ElfW( Xword ) strings_size = DynamicValue( map, DT_STRSZ );
			if( relocations == nullptr || symbols == nullptr || strings == nullptr ||
				DynamicValue( map, DT_PLTREL ) != DT_RELA )
				return nullptr;

			for( size_t i = 0; i < size / sizeof( ElfW( Rela ) ); ++i )
			{
				const ElfW( Rela ) &entry = relocations[i];
				const ElfW( Word ) name = symbols[ELF64_R_SYM( entry.r_info )].st_name;
				if( ELF64_R_TYPE( entry.r_info ) == R_X86_64_JUMP_SLOT && name < strings_size &&
					std::strcmp( strings + name, symbol ) == 0 )
					return reinterpret_cast<void **>( map->l_addr + entry.r_offset );
			}

			return nullptr;
		}
	}
}

#endif
