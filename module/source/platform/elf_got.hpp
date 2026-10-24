#pragma once

namespace Vanish
{
	/// Reads the dynamic linking tables of a loaded, position independent
	/// ELF module.
	namespace ElfGot
	{
		/// Finds the global offset table slot the module's PLT stub for an
		/// imported function jumps through. Every call the module makes to
		/// that function reads this slot, so replacing it redirects them all.
		///@param handle dlopen handle of the module
		///@param symbol name of the imported function
		///@return the slot, nullptr when the module imports no such function
		///through its PLT
		void **FindJumpSlot( void *handle, const char *symbol );
	}
}
