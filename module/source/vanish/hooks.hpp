#pragma once

namespace Vanish
{
	/// Installs and removes the two engine hooks that hide entities.
	///
	/// Neither is fatal: a failed hook warns, leaves the matching feature off
	/// and lets the rest of the addon run, because a signature that breaks on
	/// a game update must not take the server down with it.
	namespace Hooks
	{
		void Attach( );

		void Detach( );

		///@return true when entity hiding is fully installed
		bool Ready( );
	}
}
