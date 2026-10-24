#include "memory.hpp"
#include "protection.hpp"

#include <Platform.hpp>

#if defined SYSTEM_POSIX

#include <cstdint>

#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>

namespace Vanish
{
	namespace Memory
	{
		bool WritePointer( void **slot, void *value )
		{
			const uintptr_t address = reinterpret_cast<uintptr_t>( slot );
			if( address % sizeof( void * ) != 0 )
				return false;

			const int protection = Protection::Of( slot );
			if( protection == -1 )
				return false;

			// an aligned pointer never straddles two pages
			const uintptr_t page_size = static_cast<uintptr_t>( sysconf( _SC_PAGESIZE ) );
			void *page = reinterpret_cast<void *>( address & ~( page_size - 1 ) );
			const bool locked = ( protection & PROT_WRITE ) == 0;
			if( locked && mprotect( page, page_size, protection | PROT_WRITE ) != 0 )
				return false;

			// atomic, so a thread calling through the slot never sees a torn pointer
			__atomic_store_n( slot, value, __ATOMIC_SEQ_CST );

			if( locked )
				mprotect( page, page_size, protection );

			return true;
		}

		void PinModule( )
		{
			Dl_info self;
			if( dladdr( reinterpret_cast<void *>( &PinModule ), &self ) == 0 || self.dli_fname == nullptr )
				return;

			// the handle is deliberately never closed: holding it is the pin
			dlopen( self.dli_fname, RTLD_NOW | RTLD_NOLOAD | RTLD_NODELETE );
		}
	}
}

#endif
