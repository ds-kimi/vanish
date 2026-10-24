#include "rate_limit.hpp"
#include "challenge.hpp"
#include "state.hpp"

#include <chrono>
#include <unordered_map>

namespace Vanish
{
	namespace Query
	{
		namespace RateLimit
		{
			// past this many tracked addresses the expired ones are dropped, at
			// most once a second so a flood cannot make every query walk the map
			const size_t PRUNE_AT = 4096;
			const double PRUNE_INTERVAL = 1.0;

			// hard cap: a flood of spoofed source addresses would otherwise grow
			// the map without bound. Past it, new addresses only count globally
			const size_t MAX_TRACKED = 8192;

			struct Window
			{
				double start;
				int32_t count;
			};

			// keyed by the address mixed with the process secret, which is a
			// bijection, so keys stay unique while their buckets stay
			// unpredictable to whoever picks the source addresses
			static std::unordered_map<uint32_t, Window> addresses;
			static Window total = { 0.0, 0 };
			static double last_prune = 0.0;

			/// Counts one query in window, the way the engine does: a window
			/// that has run its length restarts at one.
			///@return whether the average stays within the limit
			static bool Count( Window &window, double now, double length, double limit )
			{
				if( now - window.start > length )
				{
					window.start = now;
					window.count = 1;
					return true;
				}

				if( window.count < INT32_MAX )
					++window.count;

				return window.count / length <= limit;
			}

			static void Prune( double now, double length )
			{
				if( addresses.size( ) < PRUNE_AT || now - last_prune < PRUNE_INTERVAL )
					return;

				last_prune = now;
				for( auto entry = addresses.begin( ); entry != addresses.end( ); )
					if( now - entry->second.start > length )
						entry = addresses.erase( entry );
					else
						++entry;
			}

			///@return the window of an address, nullptr when the map is full
			static Window *Track( uint32_t address, double now )
			{
				const uint32_t key = Challenge::Mix( address );
				const auto found = addresses.find( key );
				if( found != addresses.end( ) )
					return &found->second;

				if( addresses.size( ) >= MAX_TRACKED )
					return nullptr;

				return &addresses.emplace( key, Window{ now, 0 } ).first->second;
			}

			bool Allow( uint32_t address )
			{
				const ConVars &limits = Get( ).settings;
				const double length = limits.queries_window > 0.0f ? limits.queries_window : 1.0;
				const double now = std::chrono::duration<double>(
					std::chrono::steady_clock::now( ).time_since_epoch( ) ).count( );

				Prune( now, length );

				// checked first, so one address flooding does not use up the
				// global allowance as well
				Window *window = Track( address, now );
				if( window != nullptr && !Count( *window, now, length, limits.queries_per_second ) )
					return false;

				return Count( total, now, length, limits.queries_global );
			}
		}
	}
}
