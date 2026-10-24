#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Vanish
{
	/// Where a loaded ELF module sits in memory.
	namespace ElfLayout
	{
		struct Segment
		{
			uint8_t *start;
			size_t size;
			bool writable;
		};

		struct Layout
		{
			uintptr_t base;
			const char *path;
			std::vector<Segment> segments;
		};

		///@param handle dlopen handle of the module
		///@return false when the module or its PT_LOAD segments are unknown
		bool Describe( void *handle, Layout &layout );

		///@return whether [address, address + size) lies inside one segment
		bool Contains( const Layout &layout, const void *address, size_t size, bool writable );

		///@return every pointer aligned address in the segments holding value
		std::vector<uintptr_t *> FindWord( const Layout &layout, uintptr_t value, bool writable_only );
	}
}
