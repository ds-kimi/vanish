#include "hidden_set.hpp"

#include <cstring>

namespace Vanish
{
	void HiddenSet::SetHidden( int32_t entity, bool hidden )
	{
		if( entity < 1 || entity >= MAX_ENTITIES || m_hidden[entity] == hidden )
			return;

		m_hidden[entity] = hidden;
		if( hidden )
		{
			m_position[entity] = m_count;
			m_list[m_count++] = entity;
			return;
		}

		// the last entry fills the hole, so the list stays compact
		const int32_t hole = m_position[entity];
		const int32_t last = m_list[--m_count];
		m_list[hole] = last;
		m_position[last] = hole;
	}

	void HiddenSet::SetExempt( int32_t viewer, bool exempt )
	{
		if( viewer < 1 || viewer >= MAX_VIEWERS )
			return;

		m_exempt[viewer] = exempt;
	}

	void HiddenSet::Reset( )
	{
		for( int32_t i = 0; i < m_count; ++i )
			m_hidden[m_list[i]] = false;

		std::memset( m_exempt, 0, sizeof( m_exempt ) );
		m_count = 0;
	}

	HiddenSet &Hidden( )
	{
		// zero initialised as a static, which is the empty set
		static HiddenSet set;
		return set;
	}
}
