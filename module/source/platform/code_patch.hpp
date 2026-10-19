#pragma once

#include <cstddef>

namespace Vanish
{
	/// Writes into the engine's code and allocates executable stubs.
	///
	/// Only the Linux build needs this: there the engine inlines what the
	/// Windows build detours as whole functions, so the hooks have to be
	/// spliced into the middle of engine code instead.
	namespace CodePatch
	{
		/// Copies bytes over code, lifting the page protection for the write.
		///@return false when the pages could not be made writable
		bool Write( void *address, const void *bytes, size_t size );

		///@return a readable, writable and executable block, nullptr on failure
		void *Allocate( size_t size );

		void Free( void *block, size_t size );
	}
}
