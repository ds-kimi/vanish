#include "stats.hpp"
#include "game_server.hpp"

#include <iserver.h>

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

		double ClampPercent( double cpu_delta, double wall_delta )
		{
			// the first call only records a baseline: a percentage needs two
			// samples, and the engine's own figure is an interval average too
			if( !( wall_delta > 0.0 ) )
				return 0.0;

			const double percent = 100.0 * cpu_delta / wall_delta;
			return !( percent > 0.0 ) ? 0.0 : ( percent > 100.0 ? 100.0 : percent );
		}
	}
}
