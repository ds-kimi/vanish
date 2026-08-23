#pragma once

#include <cstdint>

namespace Vanish
{
	/// Reads the fields of the engine's CEntityWriteInfo that the delta header
	/// detour needs.
	///
	/// The layout was taken from CBaseServer::WriteDeltaEntities: the struct is
	/// built on its stack, the receiving client's entity index is copied out of
	/// the client at +0x10, and the deletion bitvec is the 0x400 byte field it
	/// clears with memset. Both offsets were verified against the shipping x86
	/// engine; see CONTRIBUTING.md for how to re-derive them.
	namespace EntityWriteInfo
	{
		const size_t OFFSET_CLIENT_ENTITY = 40;
		const size_t OFFSET_DELETION_FLAGS = 52;

		///@return entity index of the client this snapshot is being written for
		inline int32_t Receiver( void *info )
		{
			const uintptr_t base = reinterpret_cast<uintptr_t>( info );
			return *reinterpret_cast<int32_t *>( base + OFFSET_CLIENT_ENTITY );
		}

		/// Adds an entity to the deletion list the engine writes after the
		/// per-entity headers, mirroring what it does for a real deletion.
		inline void MarkDeleted( void *info, int32_t entity )
		{
			const uintptr_t base = reinterpret_cast<uintptr_t>( info );
			uint32_t *deletions =
				reinterpret_cast<uint32_t *>( base + OFFSET_DELETION_FLAGS );

			deletions[entity >> 5] |= 1u << ( entity & 31 );
		}
	}
}
