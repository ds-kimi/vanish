#pragma once

#include <cstdint>

namespace Vanish
{
	namespace UserInfo
	{
		/// Blanks a client's userinfo string table entry, keeping the original
		/// bytes so the entry can be put back exactly as it was.
		///@param slot client slot, the entity index minus the world entity
		///@return whether the table was reachable and the slot valid
		bool Hide( int32_t slot );

		/// Restores an entry blanked by Hide, doing nothing for a slot that was
		/// never hidden.
		///@param slot client slot
		bool Show( int32_t slot );

		/// Restores every hidden entry, for module unload.
		void Reset( );
	}
}
