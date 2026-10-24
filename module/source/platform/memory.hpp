#pragma once

namespace Vanish
{
	/// Pointer writes into memory the loader protects, such as import tables.
	namespace Memory
	{
		/// Replaces an aligned pointer in place, lifting the page protection
		/// only for the write and putting the original protection back.
		///@param slot address of the pointer to replace
		///@param value new pointer
		///@return false when the slot is misaligned or the page could not be
		///made writable, in which case nothing was written
		bool WritePointer( void **slot, void *value );

		/// Keeps this module loaded for the rest of the process.
		///
		/// Used when a hook cannot be taken back because something else has
		/// chained onto it since: unloading would leave that caller jumping
		/// into freed code.
		void PinModule( );
	}
}
