#include "protection.hpp"

#include <Platform.hpp>

#if defined SYSTEM_POSIX

#include <cinttypes>
#include <cstdint>
#include <cstdio>

#include <sys/mman.h>

namespace Vanish
{
	namespace Protection
	{
		///@return PROT_* flags spelled by the rwx field of a maps line
		static int FromFlags( const char *flags )
		{
			return ( flags[0] == 'r' ? PROT_READ : 0 ) |
				( flags[1] == 'w' ? PROT_WRITE : 0 ) |
				( flags[2] == 'x' ? PROT_EXEC : 0 );
		}

		int Of( const void *address )
		{
			FILE *maps = std::fopen( "/proc/self/maps", "r" );
			if( maps == nullptr )
				return -1;

			const uintptr_t target = reinterpret_cast<uintptr_t>( address );
			int protection = -1;

			// a line longer than the buffer is read in pieces, and the pieces
			// after the first never parse as a range, so they are skipped
			char line[512];
			while( protection == -1 && std::fgets( line, sizeof( line ), maps ) != nullptr )
			{
				uintptr_t start = 0, end = 0;
				char flags[5] = { 0 };
				if( std::sscanf( line, "%" SCNxPTR "-%" SCNxPTR " %4s", &start, &end, flags ) != 3 )
					continue;

				if( target >= start && target < end )
					protection = FromFlags( flags );
			}

			std::fclose( maps );
			return protection;
		}
	}
}

#endif
