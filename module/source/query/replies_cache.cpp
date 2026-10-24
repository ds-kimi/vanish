#include "replies_cache.hpp"

namespace Vanish
{
	namespace Query
	{
		namespace Replies
		{
			Interfaces &Engine( )
			{
				static Interfaces interfaces;
				return interfaces;
			}

			Cache &Data( )
			{
				static Cache cache;
				return cache;
			}
		}
	}
}
