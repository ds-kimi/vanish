#include "elf_module.hpp"
#include "code_patch.hpp"

#include <cstring>

#include <dlfcn.h>
#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace Vanish
{
	namespace ElfModule
	{
		// opcode of call rel32, the only form a relocated libc call takes in
		// code built without -fPIC
		const uint8_t CALL_REL32 = 0xE8;

		struct Segment
		{
			uint8_t *start;
			size_t size;
			bool writable;
		};

		struct Layout
		{
			uintptr_t base;
			const char *path;
			std::vector<Segment> segments;
		};

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

		///@return the load base, file path and mapped segments of a module
		static bool Describe( void *handle, Layout &layout )
		{
			link_map *map = nullptr;
			if( handle == nullptr || dlinfo( handle, RTLD_DI_LINKMAP, &map ) != 0 || map == nullptr )
				return false;

			layout.base = map->l_addr;
			layout.path = map->l_name;
			dl_iterate_phdr( CollectSegments, &layout );
			return !layout.segments.empty( );
		}

		///@return every pointer aligned address in the segments holding value
		static std::vector<uintptr_t *> FindWord(
			const Layout &layout, uintptr_t value, bool writable_only )
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

		///@return address of the NUL terminated type name, nullptr when absent
		static const char *FindTypeName( const Layout &layout, const char *name )
		{
			const size_t length = std::strlen( name ) + 1;
			for( const Segment &segment : layout.segments )
			{
				if( segment.writable || segment.size < length )
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

		void *FindInstance( void *handle, const char *mangled_name )
		{
			Layout layout;
			if( !Describe( handle, layout ) )
				return nullptr;

			const char *name = FindTypeName( layout, mangled_name );
			if( name == nullptr )
				return nullptr;

			// typeinfo is { vptr, name, ... }, so it starts one word before
			// the reference to its name
			const std::vector<uintptr_t *> names =
				FindWord( layout, reinterpret_cast<uintptr_t>( name ), false );
			if( names.size( ) != 1 )
				return nullptr;

			const uintptr_t typeinfo = reinterpret_cast<uintptr_t>( names[0] - 1 );

			// the primary vtable is { offset to top = 0, typeinfo, functions },
			// and objects point at its first function
			uintptr_t vtable = 0;
			for( uintptr_t *slot : FindWord( layout, typeinfo, false ) )
			{
				if( slot[-1] != 0 )
					continue;

				if( vtable != 0 )
					return nullptr;

				vtable = reinterpret_cast<uintptr_t>( slot + 1 );
			}

			if( vtable == 0 )
				return nullptr;

			const std::vector<uintptr_t *> objects = FindWord( layout, vtable, true );
			return objects.size( ) == 1 ? objects[0] : nullptr;
		}

#if defined __i386__

		/// Collects the operand of every R_386_PC32 relocation against symbol.
		static std::vector<int32_t *> FindCallOperands(
			const Layout &layout, const uint8_t *file, const char *symbol )
		{
			std::vector<int32_t *> operands;

			const Elf32_Ehdr &header = *reinterpret_cast<const Elf32_Ehdr *>( file );
			const Elf32_Shdr *sections =
				reinterpret_cast<const Elf32_Shdr *>( file + header.e_shoff );

			for( Elf32_Half i = 0; i < header.e_shnum; ++i )
			{
				const Elf32_Shdr &relocations = sections[i];
				if( relocations.sh_type != SHT_REL || relocations.sh_link >= header.e_shnum )
					continue;

				const Elf32_Shdr &symbols = sections[relocations.sh_link];
				if( symbols.sh_link >= header.e_shnum )
					continue;

				const Elf32_Sym *table = reinterpret_cast<const Elf32_Sym *>( file + symbols.sh_offset );
				const char *strings =
					reinterpret_cast<const char *>( file + sections[symbols.sh_link].sh_offset );

				const Elf32_Rel *entry = reinterpret_cast<const Elf32_Rel *>( file + relocations.sh_offset );
				const Elf32_Rel *end = entry + relocations.sh_size / sizeof( Elf32_Rel );
				for( ; entry < end; ++entry )
				{
					if( ELF32_R_TYPE( entry->r_info ) != R_386_PC32 )
						continue;

					const Elf32_Sym &target = table[ELF32_R_SYM( entry->r_info )];
					if( std::strcmp( strings + target.st_name, symbol ) != 0 )
						continue;

					uint8_t *operand = reinterpret_cast<uint8_t *>( layout.base + entry->r_offset );
					if( operand[-1] == CALL_REL32 )
						operands.push_back( reinterpret_cast<int32_t *>( operand ) );
				}
			}

			return operands;
		}

		///@return where a call rel32 whose operand is at operand lands
		static uintptr_t CallTarget( const int32_t *operand )
		{
			return reinterpret_cast<uintptr_t>( operand + 1 ) + *operand;
		}

		void *RedirectCalls(
			void *handle,
			const char *symbol,
			void *replacement,
			std::vector<CallSite> &patched )
		{
			Layout layout;
			if( !Describe( handle, layout ) || layout.path == nullptr )
				return nullptr;

			const int descriptor = open( layout.path, O_RDONLY );
			if( descriptor == -1 )
				return nullptr;

			struct stat status;
			void *file = MAP_FAILED;
			if( fstat( descriptor, &status ) == 0 )
				file = mmap( nullptr, status.st_size, PROT_READ, MAP_PRIVATE, descriptor, 0 );

			close( descriptor );
			if( file == MAP_FAILED )
				return nullptr;

			const std::vector<int32_t *> operands =
				FindCallOperands( layout, static_cast<const uint8_t *>( file ), symbol );
			munmap( file, status.st_size );

			if( operands.empty( ) )
				return nullptr;

			// every site was relocated against the same import, so a site that
			// disagrees means this is not the code the relocations describe
			const uintptr_t original = CallTarget( operands[0] );
			for( int32_t *operand : operands )
				if( CallTarget( operand ) != original )
					return nullptr;

			for( int32_t *operand : operands )
			{
				const int32_t value = static_cast<int32_t>(
					reinterpret_cast<uintptr_t>( replacement ) -
					reinterpret_cast<uintptr_t>( operand + 1 ) );

				CallSite site = { operand, *operand };
				if( !CodePatch::Write( operand, &value, sizeof( value ) ) )
				{
					Restore( patched );
					return nullptr;
				}

				patched.push_back( site );
			}

			return reinterpret_cast<void *>( original );
		}

#else

		void *RedirectCalls( void *, const char *, void *, std::vector<CallSite> & )
		{
			// a 64-bit engine is position independent and calls through its
			// PLT, which this does not handle
			return nullptr;
		}

#endif

		void Restore( std::vector<CallSite> &patched )
		{
			for( const CallSite &site : patched )
				CodePatch::Write( site.operand, &site.original, sizeof( site.original ) );

			patched.clear( );
		}
	}
}
