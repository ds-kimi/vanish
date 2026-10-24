#pragma once

namespace Vanish
{
	/// The page protection of mapped memory, which POSIX can set but not
	/// query.
	namespace Protection
	{
		///@return PROT_* flags of the mapping holding address, -1 when the
		///address is not mapped or the mappings could not be read
		int Of( const void *address );
	}
}
