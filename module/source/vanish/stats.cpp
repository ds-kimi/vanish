#include "stats.hpp"

#include <GarrysMod/InterfacePointers.hpp>

#include <iserver.h>

#if defined SYSTEM_WINDOWS
#include <Windows.h>
#endif

namespace Vanish
{
	namespace Stats
	{
		///@return the game server interface, nullptr when unavailable
		static IServer *Server( )
		{
			static IServer *server = nullptr;

			if( server == nullptr )
				server = InterfacePointers::Server( );

			return server;
		}

		bool NetStats( float &in, float &out )
		{
			IServer *server = Server( );
			if( server == nullptr )
				return false;

			server->GetNetStats( in, out );
			return true;
		}

		int SpawnCount( )
		{
			IServer *server = Server( );

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

			return 0.0;

#endif
		}
	}
}
