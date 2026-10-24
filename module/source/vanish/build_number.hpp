#pragma once

namespace Vanish
{
	/// The engine build number the status command prints.
	namespace BuildNumber
	{
		///@return the build, 0 when it cannot be found. The getter is
		///scanned for once; the value is read on every call
		int Get( );
	}
}
