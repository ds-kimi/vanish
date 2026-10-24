#include "elf_instance.hpp"
#include "elf_layout.hpp"

#include <Platform.hpp>

#if defined SYSTEM_LINUX

#include <cstring>

namespace Vanish
{
	namespace ElfInstance
	{
		///@return address of the NUL terminated type name, nullptr when absent
		static const char *FindTypeName( const ElfLayout::Layout &layout, const char *name )
		{
			const size_t length = std::strlen( name ) + 1;
			for( const ElfLayout::Segment &segment : layout.segments )
			{
				if( segment.writable || segment.size < length + 1 )
					continue;

				const uint8_t *end = segment.start + segment.size - length;
				for( const uint8_t *byte = segment.start + 1; byte <= end; ++byte )
					// the byte before must end another string, or this is
					// only the tail of a longer name such as a nested class
					if( byte[-1] == 0 && std::memcmp( byte, name, length ) == 0 )
						return reinterpret_cast<const char *>( byte );
			}

			return nullptr;
		}

		///@return the primary vtable's first function slot, 0 when not unique
		static uintptr_t FindVTable( const ElfLayout::Layout &layout, uintptr_t typeinfo )
		{
			// the primary vtable is { offset to top = 0, typeinfo, functions },
			// and objects point at its first function
			uintptr_t vtable = 0;
			for( uintptr_t *slot : ElfLayout::FindWord( layout, typeinfo, false ) )
			{
				const bool mapped = ElfLayout::Contains( layout, slot - 1, sizeof( *slot ), false ) ||
					ElfLayout::Contains( layout, slot - 1, sizeof( *slot ), true );
				if( !mapped || slot[-1] != 0 )
					continue;

				if( vtable != 0 )
					return 0;

				vtable = reinterpret_cast<uintptr_t>( slot + 1 );
			}

			return vtable;
		}

		void *Find( void *handle, const char *mangled_name )
		{
			ElfLayout::Layout layout;
			if( !ElfLayout::Describe( handle, layout ) )
				return nullptr;

			const char *name = FindTypeName( layout, mangled_name );
			if( name == nullptr )
				return nullptr;

			// typeinfo is { vptr, name, ... }, so it starts one word before
			// the reference to its name
			const std::vector<uintptr_t *> names =
				ElfLayout::FindWord( layout, reinterpret_cast<uintptr_t>( name ), false );
			if( names.size( ) != 1 )
				return nullptr;

			const uintptr_t vtable =
				FindVTable( layout, reinterpret_cast<uintptr_t>( names[0] - 1 ) );
			if( vtable == 0 )
				return nullptr;

			const std::vector<uintptr_t *> objects = ElfLayout::FindWord( layout, vtable, true );
			return objects.size( ) == 1 ? objects[0] : nullptr;
		}
	}
}

#endif
