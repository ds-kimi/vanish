#pragma once

namespace Vanish
{
	/// Finds polymorphic globals a Windows module does not export.
	namespace PeInstance
	{
		/// Finds the global instance of a polymorphic class through MSVC RTTI:
		/// the decorated name leads to the type descriptor, the descriptor to
		/// the complete object locator of the primary vtable, the locator to
		/// the vtable, and the vtable to the one object pointing at it.
		///
		/// Objects in .bss only receive their vtable when constructed, so this
		/// finds them at runtime where a scan of the file cannot.
		///@param module handle of the module
		///@param decorated_name MSVC type name, such as ".?AVCGameServer@@"
		///@return the object, nullptr when any step is ambiguous or missing
		void *Find( void *module, const char *decorated_name );
	}
}
