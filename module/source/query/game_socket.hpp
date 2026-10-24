#pragma once

#include "recv_hook.hpp"

#include <cstdint>

namespace Vanish
{
	namespace Query
	{
		/// Tells the game socket apart from the engine's other sockets.
		///
		/// Every socket the engine reads goes through the same hook, and only
		/// the one bound to the game port receives the queries this answers.
		/// The socket is recognised by its bound port rather than through the
		/// engine's socket table, whose layout and symbol differ per build.
		namespace GameSocket
		{
			/// Sets the port the game socket is bound to and forgets the socket
			/// recognised so far, in case the engine reopened it.
			///@param port in host order
			void SetPort( uint16_t port );

			///@return whether socket is bound to the game port
			bool Is( RecvHook::Socket socket );
		}
	}
}
