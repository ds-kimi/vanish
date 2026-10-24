#include "stats.hpp"

#include <Platform.hpp>

#if defined SYSTEM_POSIX

#include <time.h>

namespace Vanish
{
	namespace Stats
	{
		static double Seconds( const timespec &time )
		{
			return static_cast<double>( time.tv_sec ) + static_cast<double>( time.tv_nsec ) / 1e9;
		}

		double CPUUsage( )
		{
			static double last_wall = 0.0, last_cpu = 0.0;

			timespec wall_time, cpu_time;
			if( clock_gettime( CLOCK_MONOTONIC, &wall_time ) != 0 ||
				clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &cpu_time ) != 0 )
				return 0.0;

			const double wall = Seconds( wall_time );
			const double cpu = Seconds( cpu_time );
			const bool primed = last_wall != 0.0 && wall > last_wall;
			const double wall_delta = primed ? wall - last_wall : 0.0;
			const double cpu_delta = primed ? cpu - last_cpu : 0.0;

			last_wall = wall;
			last_cpu = cpu;

			return ClampPercent( cpu_delta, wall_delta );
		}
	}
}

#endif
