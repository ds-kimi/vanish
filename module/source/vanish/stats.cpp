#include "stats.hpp"
#include "game_server.hpp"

#include <Platform.hpp>

#include <iserver.h>

#if defined SYSTEM_WINDOWS
#include <Windows.h>
#else
#include <time.h>
#endif

namespace Vanish
{
	namespace Stats
	{
		bool NetStats( float &in, float &out )
		{
			IServer *server = GameServer::Get( );
			if( server == nullptr )
				return false;

			server->GetNetStats( in, out );
			return true;
		}

		int SpawnCount( )
		{
			IServer *server = GameServer::Get( );

			return server != nullptr ? server->GetSpawnCount( ) : -1;
		}

		double CPUUsage( )
		{
#if defined SYSTEM_WINDOWS

			static ULONGLONG last_wall = 0, last_cpu = 0;

			FILETIME creation, exit, kernel, user, now;
			if( GetProcessTimes( GetCurrentProcess( ), &creation, &exit, &kernel, &user ) == 0 )
				return 0.0;

			GetSystemTimeAsFileTime( &now );

			const auto pack = []( const FILETIME &time ) -> ULONGLONG {
				return ( static_cast<ULONGLONG>( time.dwHighDateTime ) << 32 ) | time.dwLowDateTime;
			};

			const ULONGLONG wall = pack( now );
			const ULONGLONG cpu = pack( kernel ) + pack( user );

			// the first call only records a baseline: a percentage needs two
			// samples, and the engine's own figure is an interval average too
			const ULONGLONG wall_delta = wall - last_wall;
			const ULONGLONG cpu_delta = cpu - last_cpu;
			const bool primed = last_wall != 0 && wall > last_wall;

			last_wall = wall;
			last_cpu = cpu;

			if( !primed )
				return 0.0;

			const double percent = 100.0 * static_cast<double>( cpu_delta ) /
				static_cast<double>( wall_delta );

			return percent < 0.0 ? 0.0 : ( percent > 100.0 ? 100.0 : percent );

#else

			static double last_wall = 0.0, last_cpu = 0.0;

			timespec wall_time, cpu_time;
			if( clock_gettime( CLOCK_MONOTONIC, &wall_time ) != 0 ||
				clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &cpu_time ) != 0 )
				return 0.0;

			const auto seconds = []( const timespec &time ) -> double {
				return static_cast<double>( time.tv_sec ) + time.tv_nsec / 1e9;
			};

			const double wall = seconds( wall_time );
			const double cpu = seconds( cpu_time );

			// same baseline rule as the Windows branch above
			const double wall_delta = wall - last_wall;
			const double cpu_delta = cpu - last_cpu;
			const bool primed = last_wall != 0.0 && wall > last_wall;

			last_wall = wall;
			last_cpu = cpu;

			if( !primed )
				return 0.0;

			const double percent = 100.0 * cpu_delta / wall_delta;

			return percent < 0.0 ? 0.0 : ( percent > 100.0 ? 100.0 : percent );

#endif
		}
	}
}
