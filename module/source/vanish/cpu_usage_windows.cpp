#include "stats.hpp"

#include <Platform.hpp>

#if defined SYSTEM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Vanish
{
	namespace Stats
	{
		static ULONGLONG Pack( const FILETIME &time )
		{
			return ( static_cast<ULONGLONG>( time.dwHighDateTime ) << 32 ) | time.dwLowDateTime;
		}

		double CPUUsage( )
		{
			static ULONGLONG last_wall = 0, last_cpu = 0;

			FILETIME creation, exit, kernel, user, now;
			if( GetProcessTimes( GetCurrentProcess( ), &creation, &exit, &kernel, &user ) == 0 )
				return 0.0;

			GetSystemTimeAsFileTime( &now );

			const ULONGLONG wall = Pack( now );
			const ULONGLONG cpu = Pack( kernel ) + Pack( user );
			const bool primed = last_wall != 0 && wall > last_wall && cpu >= last_cpu;
			const double wall_delta = primed ? static_cast<double>( wall - last_wall ) : 0.0;
			const double cpu_delta = primed ? static_cast<double>( cpu - last_cpu ) : 0.0;

			last_wall = wall;
			last_cpu = cpu;

			return ClampPercent( cpu_delta, wall_delta );
		}
	}
}

#endif
