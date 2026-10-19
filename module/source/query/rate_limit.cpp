#include "rate_limit.hpp"
#include "state.hpp"

#include <Platform.hpp>

#include <chrono>
#include <cstdint>
#include <unordered_map>

#if defined SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

namespace Vanish
{
	namespace Query
	{
		namespace RateLimit
		{
			// past this many tracked addresses, the expired ones are dropped
			const size_t PRUNE_AT = 4096;

			struct Window
			{
				double start;
				int32_t count;
			};

			static std::unordered_map<uint32_t, Window> addresses;
			static Window total = { 0.0, 0 };

			static double Now( )
			{
				return std::chrono::duration<double>(
					std::chrono::steady_clock::now( ).time_since_epoch( ) ).count( );
			}

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

				++window.count;
				return window.count / length <= limit;
			}

			static void Prune( double now, double length )
			{
				for( auto entry = addresses.begin( ); entry != addresses.end( ); )
					if( now - entry->second.start > length )
						entry = addresses.erase( entry );
					else
						++entry;
			}

			bool Allow( const sockaddr *from )
			{
				const ConVars &limits = Get( ).Settings( );
				const double length = limits.queries_window > 0.0f ? limits.queries_window : 1.0;
				const double now = Now( );

				if( from != nullptr && from->sa_family == AF_INET )
				{
					if( addresses.size( ) >= PRUNE_AT )
						Prune( now, length );

					const uint32_t address = reinterpret_cast<const sockaddr_in *>( from )->sin_addr.s_addr;
					Window &window = addresses.emplace( address, Window{ now, 0 } ).first->second;

					// checked first, so one address flooding does not use up
					// the global allowance as well
					if( !Count( window, now, length, limits.queries_per_second ) )
						return false;
				}

				return Count( total, now, length, limits.queries_global );
			}
		}
	}
}
