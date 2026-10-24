#include "hidden_set.hpp"

namespace Vanish
{
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
}
