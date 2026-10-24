#include "hidden_set.hpp"

namespace Vanish
{
	// both comparisons fail for NaN, so a NaN index is rejected rather than
	// cast, which would be undefined
	bool HiddenSet::IsEntityIndex( double value )
	{
		return value >= 1 && value < MAX_ENTITIES;
	}

	bool HiddenSet::IsViewerIndex( double value )
	{
		return value >= 1 && value < MAX_VIEWERS;
	}
}
