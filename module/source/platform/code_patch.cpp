#include "code_patch.hpp"

#include <cstdint>
#include <cstring>

#include <sys/mman.h>
#include <unistd.h>

namespace Vanish
{
	namespace CodePatch
	{
		bool Write( void *address, const void *bytes, size_t size )
		{
			const uintptr_t page = static_cast<uintptr_t>( sysconf( _SC_PAGESIZE ) );
			const uintptr_t start = reinterpret_cast<uintptr_t>( address ) & ~( page - 1 );
			const uintptr_t end = reinterpret_cast<uintptr_t>( address ) + size;
			const size_t length = end - start;

			void *pages = reinterpret_cast<void *>( start );
			if( mprotect( pages, length, PROT_READ | PROT_WRITE | PROT_EXEC ) != 0 )
				return false;

			std::memcpy( address, bytes, size );

			// engine code is mapped read and execute, so that is what it goes
			// back to rather than staying writable for the rest of the process
			mprotect( pages, length, PROT_READ | PROT_EXEC );
			__builtin___clear_cache(
				static_cast<char *>( address ), static_cast<char *>( address ) + size );

			return true;
		}

		void *Allocate( size_t size )
		{
			void *block = mmap( nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );

			return block != MAP_FAILED ? block : nullptr;
		}

		void Free( void *block, size_t size )
		{
			if( block != nullptr )
				munmap( block, size );
		}
	}
}
