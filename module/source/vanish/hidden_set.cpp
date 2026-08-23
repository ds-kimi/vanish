#include "hidden_set.hpp"

#include <cstring>

namespace Vanish
{
	void HiddenSet::SetHidden( int32_t entity, bool hidden )
	{
		if( entity < 1 || entity >= MAX_ENTITIES )
			return;

		if( m_hidden[entity] == hidden )
			return;

		m_hidden[entity] = hidden;
		RebuildList( );
	}

	void HiddenSet::SetExempt( int32_t viewer, bool exempt )
	{
		if( viewer < 1 || viewer >= MAX_VIEWERS )
			return;

		m_exempt[viewer] = exempt;
	}

	void HiddenSet::Reset( )
	{
		std::memset( m_hidden, 0, sizeof( m_hidden ) );
		std::memset( m_exempt, 0, sizeof( m_exempt ) );
		m_count = 0;
	}

	bool HiddenSet::IsHidden( int32_t entity ) const
	{
		return entity > 0 && entity < MAX_ENTITIES && m_hidden[entity];
	}

	bool HiddenSet::IsExempt( int32_t viewer ) const
	{
		return viewer > 0 && viewer < MAX_VIEWERS && m_exempt[viewer];
	}

	int32_t HiddenSet::Count( ) const
	{
		return m_count;
	}

	int32_t HiddenSet::Entity( int32_t position ) const
	{
		return position >= 0 && position < m_count ? m_list[position] : 0;
	}

	bool HiddenSet::IsEntityIndex( double value )
	{
		return value >= 1 && value < MAX_ENTITIES;
	}

	bool HiddenSet::IsViewerIndex( double value )
	{
		return value >= 1 && value < MAX_VIEWERS;
	}

	// Walking 8192 slots per client per snapshot would be wasteful, so the set
	// is flattened here instead: this runs only when the set actually changes.
	void HiddenSet::RebuildList( )
	{
		m_count = 0;
		for( int32_t i = 1; i < MAX_ENTITIES; ++i )
			if( m_hidden[i] )
				m_list[m_count++] = i;
	}

	HiddenSet &Hidden( )
	{
		static HiddenSet set;
		return set;
	}
}
