#pragma once

#include <cstdint>
#include <vector>

namespace Vanish
{
	/// Redirects the direct calls a 32-bit, non-PIC module makes to an import.
	///
	/// The 32-bit engine is built without -fPIC, so it calls libc through
	/// relocated call rel32 instructions instead of a PLT. The relocations
	/// in the file name the callee of every one of them.
	namespace ElfCalls
	{
		/// One relocated call instruction that was pointed somewhere else.
		struct CallSite
		{
			int32_t *operand;
			int32_t original;
		};

		///@return the operand of every call rel32 the file relocates against
		///symbol, mapped into the loaded module at base
		std::vector<int32_t *> FindOperands( void *handle, const char *symbol );

		/// Points every call the module makes to an imported function at a
		/// replacement, leaving calls from any other module alone.
		///@param handle dlopen handle of the module
		///@param symbol name of the imported function
		///@param replacement function to call instead
		///@param patched filled with every rewritten site, for Restore
		///@return the function the calls went to before, nullptr when no
		///call site was found or one could not be rewritten
		void *Redirect( void *handle, const char *symbol, void *replacement, std::vector<CallSite> &patched );

		/// Puts back call sites rewritten by Redirect.
		void Restore( std::vector<CallSite> &patched );
	}
}
