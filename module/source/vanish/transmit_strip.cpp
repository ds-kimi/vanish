#include "transmit_strip.hpp"
#include "hidden_set.hpp"

#include <eiface.h>
#include <iservernetworkable.h>

namespace Vanish
{
	namespace TransmitStrip
	{
		int32_t ReceiverOf( IVEngineServer *engine, const CCheckTransmitInfo *info )
		{
			if( engine == nullptr || info == nullptr || info->m_pClientEnt == nullptr )
				return 0;

			const int32_t index = engine->IndexOfEdict( info->m_pClientEnt );
			return HiddenSet::IsViewerIndex( index ) ? index : 0;
		}

		void StripHidden( CCheckTransmitInfo *info, int32_t receiver )
		{
			// CBitVec<MAX_EDICTS> stores its bits as 32-bit words on every build
			uint32_t *transmit = reinterpret_cast<uint32_t *>( info->m_pTransmitEdict );
			uint32_t *always = reinterpret_cast<uint32_t *>( info->m_pTransmitAlways );

			const HiddenSet &hidden = Hidden( );
			for( int32_t i = 0; i < hidden.Count( ); ++i )
			{
				const int32_t entity = hidden.Entity( i );
				if( entity == receiver )
					continue;

				if( transmit != nullptr )
					transmit[entity >> 5] &= ~( 1u << ( entity & 31 ) );

				if( always != nullptr )
					always[entity >> 5] &= ~( 1u << ( entity & 31 ) );
			}
		}
	}
}
