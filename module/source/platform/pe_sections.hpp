#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Vanish
{
	/// The sections of a loaded Windows module, as mapped in memory.
	namespace PeSections
	{
		struct Section
		{
			uint8_t *start;
			size_t size;
			bool writable;
		};

		///@return every section of the module, empty when the headers are invalid
		std::vector<Section> Of( void *module );

		///@return every aligned address in the sections holding value; with
		///writable_only, only sections the module can write
		std::vector<uintptr_t *> FindWord( const std::vector<Section> &sections, uintptr_t value, bool writable_only );

		///@return the first occurrence of bytes in any section, nullptr when absent
		const uint8_t *FindBytes( const std::vector<Section> &sections, const void *bytes, size_t size );
	}
}
