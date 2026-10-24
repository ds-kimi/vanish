#include "elf_layout.hpp"

#include <Platform.hpp>

#if defined SYSTEM_LINUX

#include <dlfcn.h>
#include <link.h>

namespace Vanish
{
	namespace ElfLayout
	{
		static int CollectSegments( dl_phdr_info *info, size_t, void *data )
		{
			Layout &layout = *static_cast<Layout *>( data );
			if( info->dlpi_addr != layout.base )
				return 0;

			for( ElfW( Half ) i = 0; i < info->dlpi_phnum; ++i )
			{
				const ElfW( Phdr ) &header = info->dlpi_phdr[i];
				if( header.p_type != PT_LOAD )
					continue;

				Segment segment;
				segment.start = reinterpret_cast<uint8_t *>( layout.base + header.p_vaddr );
				segment.size = header.p_memsz;
				segment.writable = ( header.p_flags & PF_W ) != 0;
				layout.segments.push_back( segment );
			}

			return 1;
		}

		bool Describe( void *handle, Layout &layout )
		{
			link_map *map = nullptr;
			if( handle == nullptr || dlinfo( handle, RTLD_DI_LINKMAP, &map ) != 0 || map == nullptr )
				return false;

			layout.base = map->l_addr;
			layout.path = map->l_name;
			dl_iterate_phdr( CollectSegments, &layout );
			return !layout.segments.empty( );
		}

		bool Contains( const Layout &layout, const void *address, size_t size, bool writable )
		{
			const uint8_t *first = static_cast<const uint8_t *>( address );
			for( const Segment &segment : layout.segments )
				if( segment.writable == writable && first >= segment.start &&
					size <= segment.size && first - segment.start <= static_cast<ptrdiff_t>( segment.size - size ) )
					return true;

			return false;
		}

		std::vector<uintptr_t *> FindWord( const Layout &layout, uintptr_t value, bool writable_only )
		{
			std::vector<uintptr_t *> found;
			for( const Segment &segment : layout.segments )
			{
				if( writable_only && !segment.writable )
					continue;

				uintptr_t *word = reinterpret_cast<uintptr_t *>( segment.start );
				uintptr_t *end = word + segment.size / sizeof( uintptr_t );
				for( ; word < end; ++word )
					if( *word == value )
						found.push_back( word );
			}

			return found;
		}
	}
}

#endif
