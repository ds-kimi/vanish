#pragma once

#include <cstdint>

namespace Vanish
{
	namespace Query
	{
		/// The engine's own query rate limit, applied to the queries the
		/// filter answers in its place.
		///
		/// The engine stops answering an address once it averages more than
		/// sv_max_queries_sec over sv_max_queries_window, and everybody once
		/// the total passes sv_max_queries_sec_global. Answering past that
		/// would make filtered replies distinguishable from the engine's, and
		/// would answer a flood the engine would have ignored.
		namespace RateLimit
		{
			/// Counts a query and reports whether it may be answered. Must be
			/// called with the query state lock held.
			///@param address IPv4 address in network order
			bool Allow( uint32_t address );
		}
	}
}
