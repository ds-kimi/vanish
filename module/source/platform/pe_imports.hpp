#pragma once

#include <cstdint>

namespace Vanish
{
	/// Reads the import tables of a loaded Windows module.
	namespace PeImports
	{
		/// Finds the import address table slot a module calls an imported
		/// function through. Every call the module makes to that function
		/// reads this slot, so replacing it redirects them all.
		///@param module handle of the importing module
		///@param library name of the exporting DLL, compared case insensitively
		///@param function exported name
		///@param ordinal exported ordinal, for imports made by ordinal
		///@return the slot, nullptr when the module does not import it
		void **FindSlot( void *module, const char *library, const char *function, uint16_t ordinal );
	}
}
