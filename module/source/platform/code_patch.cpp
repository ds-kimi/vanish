#include "code_patch.hpp"
#include "protection.hpp"

#include <Platform.hpp>

#if defined SYSTEM_POSIX

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
			const int protection = Protection::Of( address );
			if( protection == -1 )
				return false;

			const uintptr_t page = static_cast<uintptr_t>( sysconf( _SC_PAGESIZE ) );
			const uintptr_t start = reinterpret_cast<uintptr_t>( address ) & ~( page - 1 );
			const uintptr_t end = reinterpret_cast<uintptr_t>( address ) + size;

			void *pages = reinterpret_cast<void *>( start );
			if( mprotect( pages, end - start, PROT_READ | PROT_WRITE | PROT_EXEC ) != 0 )
				return false;

			std::memcpy( address, bytes, size );

			// back to what the loader mapped, rather than leaving engine code
			// writable for the rest of the process
			mprotect( pages, end - start, protection );
			__builtin___clear_cache(
				static_cast<char *>( address ), static_cast<char *>( address ) + size );

			return true;
		}

		void *Allocate( size_t size )
		{
			void *block = mmap( nullptr, size, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );

			return block != MAP_FAILED ? block : nullptr;
		}

		bool Seal( void *block, size_t size )
		{
			if( mprotect( block, size, PROT_READ | PROT_EXEC ) != 0 )
				return false;

			__builtin___clear_cache(
				static_cast<char *>( block ), static_cast<char *>( block ) + size );
			return true;
		}

		void Free( void *block, size_t size )
		{
			if( block != nullptr )
				munmap( block, size );
		}
	}
}

#endif
