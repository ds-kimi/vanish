#pragma once

#include <cstdint>

namespace Vanish
{
	// Weapons, ragdolls and children live far above the player slots, so the
	// hidden set has to span the whole edict range.
	const int32_t MAX_ENTITIES = 8192;

	// Only players ever receive a snapshot, so viewer state stays small.
	const int32_t MAX_VIEWERS = 129;

	/// Which entities are hidden, and which viewers are allowed to see them.
	///
	/// Both hooks read this on the server's networking path, once per client
	/// per snapshot, so lookups are plain array reads and the hidden entities
	/// are also kept as a compact list to iterate. The list is kept up to date
	/// in constant time per change, because Lua rebuilds the whole set on
	/// every refresh, one entity at a time.
	class HiddenSet
	{
	public:
		void SetHidden( int32_t entity, bool hidden );
		void SetExempt( int32_t viewer, bool exempt );
		void Reset( );

		bool IsHidden( int32_t entity ) const;
		bool IsExempt( int32_t viewer ) const;

		int32_t Count( ) const;
		int32_t Entity( int32_t position ) const;

		static bool IsEntityIndex( double value );
		static bool IsViewerIndex( double value );

	private:
		bool m_hidden[MAX_ENTITIES];
		bool m_exempt[MAX_VIEWERS];

		// m_list holds the hidden entities in no particular order, and
		// m_position where each of them sits in it
		int32_t m_list[MAX_ENTITIES];
		int32_t m_position[MAX_ENTITIES];
		int32_t m_count;
	};

	/// The single set shared by the hooks and the Lua bindings.
	HiddenSet &Hidden( );
}
