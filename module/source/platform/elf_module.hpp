#pragma once

#include <cstdint>
#include <vector>

namespace Vanish
{
	/// Reads a loaded ELF module, for what the Linux engine does not export.
	///
	/// The 32-bit engine is built without -fPIC, so it calls libc directly
	/// through relocated call instructions and keeps no symbols for its
	/// globals. Both are still reachable: the relocations name the callee,
	/// and RTTI names the class of every polymorphic global.
	namespace ElfModule
	{
		/// One relocated call instruction that was pointed somewhere else.
		struct CallSite
		{
			int32_t *operand;
			int32_t original;
		};

		/// Points every call the module makes to an imported function at a
		/// replacement, leaving calls from any other module alone.
		///@param handle dlopen handle of the module
		///@param symbol name of the imported function
		///@param replacement function to call instead
		///@param patched filled with every rewritten site, for Restore
		///@return the function the calls went to before, nullptr when no
		///call site was found or one could not be rewritten
		void *RedirectCalls(
			void *handle,
			const char *symbol,
			void *replacement,
			std::vector<CallSite> &patched );

		/// Puts back call sites rewritten by RedirectCalls.
		void Restore( std::vector<CallSite> &patched );

		/// Finds the global instance of a polymorphic class through its RTTI:
		/// the type name leads to the typeinfo, the typeinfo to the vtable,
		/// and the vtable to the one object in the module that points at it.
		///@param handle dlopen handle of the module
		///@param mangled_name Itanium type name, such as "11CGameServer"
		///@return the object, nullptr when any step is ambiguous or missing
		void *FindInstance( void *handle, const char *mangled_name );
	}
}
