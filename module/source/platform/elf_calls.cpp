#include "elf_calls.hpp"
#include "elf_layout.hpp"

#include <Platform.hpp>

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86

#include <cstring>

#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace Vanish
{
	namespace ElfCalls
	{
		// opcode of call rel32, the only form a relocated libc call takes in
		// code built without -fPIC
		const uint8_t CALL_REL32 = 0xE8;

		///@return whether [offset, offset + length) lies inside the file
		static bool InFile( size_t file_size, size_t offset, size_t length )
		{
			return offset <= file_size && length <= file_size - offset;
		}

		/// Collects the R_386_PC32 call operands of one SHT_REL section.
		static void ScanSection( const ElfLayout::Layout &layout, const uint8_t *file, size_t size,
			const Elf32_Shdr &relocations, const Elf32_Shdr &symbols, const Elf32_Shdr &strings,
			const char *symbol, std::vector<int32_t *> &operands )
		{
			if( !InFile( size, relocations.sh_offset, relocations.sh_size ) ||
				!InFile( size, symbols.sh_offset, symbols.sh_size ) ||
				!InFile( size, strings.sh_offset, strings.sh_size ) )
				return;

			const Elf32_Sym *table = reinterpret_cast<const Elf32_Sym *>( file + symbols.sh_offset );
			const size_t symbol_count = symbols.sh_size / sizeof( Elf32_Sym );
			const Elf32_Rel *entry = reinterpret_cast<const Elf32_Rel *>( file + relocations.sh_offset );
			const Elf32_Rel *end = entry + relocations.sh_size / sizeof( Elf32_Rel );
			for( ; entry < end; ++entry )
			{
				const size_t index = ELF32_R_SYM( entry->r_info );
				if( ELF32_R_TYPE( entry->r_info ) != R_386_PC32 || index >= symbol_count ||
					table[index].st_name >= strings.sh_size )
					continue;

				const char *name = reinterpret_cast<const char *>( file + strings.sh_offset + table[index].st_name );
				const size_t length = std::strlen( symbol ) + 1;
				uint8_t *operand = reinterpret_cast<uint8_t *>( layout.base + entry->r_offset );
				if( strings.sh_size - table[index].st_name >= length && std::memcmp( name, symbol, length ) == 0 &&
					ElfLayout::Contains( layout, operand - 1, 1 + sizeof( int32_t ), false ) &&
					operand[-1] == CALL_REL32 )
					operands.push_back( reinterpret_cast<int32_t *>( operand ) );
			}
		}

		/// Walks the section headers of the module's file for SHT_REL sections.
		static std::vector<int32_t *> ScanFile( const ElfLayout::Layout &layout, const uint8_t *file,
			size_t size, const char *symbol )
		{
			std::vector<int32_t *> operands;
			if( size < sizeof( Elf32_Ehdr ) )
				return operands;

			const Elf32_Ehdr &header = *reinterpret_cast<const Elf32_Ehdr *>( file );
			if( std::memcmp( header.e_ident, ELFMAG, SELFMAG ) != 0 ||
				!InFile( size, header.e_shoff, header.e_shnum * sizeof( Elf32_Shdr ) ) )
				return operands;

			const Elf32_Shdr *sections = reinterpret_cast<const Elf32_Shdr *>( file + header.e_shoff );
			for( Elf32_Half i = 0; i < header.e_shnum; ++i )
			{
				const Elf32_Shdr &relocations = sections[i];
				if( relocations.sh_type != SHT_REL || relocations.sh_link >= header.e_shnum )
					continue;

				const Elf32_Shdr &symbols = sections[relocations.sh_link];
				if( symbols.sh_link < header.e_shnum )
					ScanSection( layout, file, size, relocations, symbols,
						sections[symbols.sh_link], symbol, operands );
			}

			return operands;
		}

		std::vector<int32_t *> FindOperands( void *handle, const char *symbol )
		{
			ElfLayout::Layout layout;
			if( !ElfLayout::Describe( handle, layout ) || layout.path == nullptr )
				return { };

			const int descriptor = open( layout.path, O_RDONLY | O_CLOEXEC );
			if( descriptor == -1 )
				return { };

			// the size comes from lseek rather than fstat: the 32-bit stat
			// fails with EOVERFLOW on filesystems with 64-bit inode numbers,
			// such as XFS with inode64, and the hook silently went missing there
			const off_t end = lseek( descriptor, 0, SEEK_END );
			void *file = MAP_FAILED;
			if( end > 0 )
				file = mmap( nullptr, static_cast<size_t>( end ), PROT_READ, MAP_PRIVATE, descriptor, 0 );

			close( descriptor );
			if( file == MAP_FAILED )
				return { };

			const size_t size = static_cast<size_t>( end );
			std::vector<int32_t *> operands = ScanFile( layout, static_cast<const uint8_t *>( file ), size, symbol );
			munmap( file, size );
			return operands;
		}
	}
}

#endif
