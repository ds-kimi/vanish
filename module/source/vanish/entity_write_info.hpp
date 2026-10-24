#pragma once

#include <Platform.hpp>

#include <cstddef>
#include <cstdint>

namespace Vanish
{
	/// Reads the fields of the engine's CEntityWriteInfo that the delta header
	/// hook needs.
	///
	/// The layout was taken from CBaseServer::WriteDeltaEntities: the struct is
	/// built on its stack, the receiving client's entity index is copied into
	/// it from the client, and the deletion bitvec is the 0x400 byte field it
	/// clears with memset. The pointer members before them make the offsets
	/// differ per architecture; both were verified against the engines of
	/// every supported build, see CONTRIBUTING.md for how to re-derive them.
	namespace EntityWriteInfo
	{
#if defined ARCHITECTURE_X86_64
		const size_t OFFSET_BUFFER = 0x38;
		const size_t OFFSET_CLIENT_ENTITY = 0x40;
		const size_t OFFSET_DELETION_FLAGS = 0x58;
#else
		const size_t OFFSET_BUFFER = 0x24;
		const size_t OFFSET_CLIENT_ENTITY = 0x28;
		const size_t OFFSET_DELETION_FLAGS = 0x34;
#endif

		// CBitVec<MAX_EDICTS>: one bit per edict
		const int32_t DELETION_BITS = 8192;

		///@return entity index of the client this snapshot is being written for
		inline int32_t Receiver( const void *info )
		{
			const uintptr_t base = reinterpret_cast<uintptr_t>( info );
			return *reinterpret_cast<const int32_t *>( base + OFFSET_CLIENT_ENTITY );
		}

		/// Adds an entity to the deletion list the engine writes after the
		/// per-entity headers, mirroring what it does for a real deletion.
		inline void MarkDeleted( void *info, int32_t entity )
		{
			if( entity < 0 || entity >= DELETION_BITS )
				return;

			const uintptr_t base = reinterpret_cast<uintptr_t>( info );
			uint32_t *deletions = reinterpret_cast<uint32_t *>( base + OFFSET_DELETION_FLAGS );
			deletions[entity >> 5] |= 1u << ( entity & 31 );
		}
	}
}
