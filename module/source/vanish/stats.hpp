#pragma once

namespace Vanish
{
	namespace Stats
	{
		/// Reads the server's averaged network throughput.
		///@param in filled with the inbound rate in KB/s
		///@param out filled with the outbound rate in KB/s
		///@return whether the server interface was reachable
		bool NetStats( float &in, float &out );

		///@return number of level loads since the process started, -1 when unknown
		int SpawnCount( );

		/// Process CPU time as a percentage of one core, measured between calls.
		///@return percentage, 0 on the first call and when the query fails
		double CPUUsage( );
	}
}
