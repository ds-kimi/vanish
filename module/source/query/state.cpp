#include "state.hpp"

namespace Vanish
{
	namespace Query
	{
		std::mutex &Lock( )
		{
			static std::mutex mutex;
			return mutex;
		}

		State &Get( )
		{
			static State state;
			return state;
		}

		std::atomic<bool> &Enabled( )
		{
			static std::atomic<bool> enabled( false );
			return enabled;
		}
	}
}
