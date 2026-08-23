#pragma once

namespace Vanish
{
	namespace Query
	{
		/// Answers A2S_INFO and A2S_PLAYER before the engine sees them.
		///
		/// The detour stays installed for the lifetime of the module and checks
		/// the state on each packet, rather than swapping the engine's function
		/// pointer whenever hiding is toggled.
		namespace SocketFilter
		{
			///@return true when the detour is installed
			bool Attach( );

			void Detach( );
		}
	}
}
