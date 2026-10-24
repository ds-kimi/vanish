#pragma once

namespace Vanish
{
	/// Finds polymorphic globals the 32-bit Linux engine keeps no symbol for.
	namespace ElfInstance
	{
		/// Finds the global instance of a polymorphic class through its RTTI:
		/// the type name leads to the typeinfo, the typeinfo to the vtable,
		/// and the vtable to the one object in the module that points at it.
		///
		/// Only valid for code built without -fPIC, where those references are
		/// absolute words in the image rather than relocations.
		///@param handle dlopen handle of the module
		///@param mangled_name Itanium type name, such as "11CGameServer"
		///@return the object, nullptr when any step is ambiguous or missing
		void *Find( void *handle, const char *mangled_name );
	}
}
