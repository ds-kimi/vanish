#include "manager.hpp"

#include "../main.hpp"

#include <igameevents.h>

namespace Vanish
{
	namespace Event
	{
		static IGameEventManager2 *manager = nullptr;

		IGameEventManager2 *Manager( )
		{
			if( manager == nullptr )
				manager = global::engine_loader.GetInterface<IGameEventManager2>(
					INTERFACEVERSION_GAMEEVENTSMANAGER2 );

			return manager;
		}
	}
}
