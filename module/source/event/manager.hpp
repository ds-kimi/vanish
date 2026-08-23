#pragma once

class IGameEventManager2;

namespace Vanish
{
	namespace Event
	{
		/// Resolves GAMEEVENTSMANAGER002 from the engine, cached after the
		/// first successful lookup.
		///@return event manager, nullptr when the engine does not expose it
		IGameEventManager2 *Manager( );
	}
}
