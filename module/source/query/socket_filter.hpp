#pragma once

namespace Vanish
{
	namespace Query
	{
		/// Answers A2S_INFO and A2S_PLAYER before the engine sees them.
		///
		/// The hook stays installed for the lifetime of the module and checks
		/// the state on each packet, rather than swapping the engine's function
		/// pointer whenever hiding is toggled.
		namespace SocketFilter
		{
			///@return true when the hook is installed
			bool Attach( );

			/// Removes the hook, or leaves it as a pass through and pins the
			/// module when another hook has chained onto it since.
			void Detach( );
		}
	}
}
