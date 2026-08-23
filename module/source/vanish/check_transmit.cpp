#include "check_transmit.hpp"
#include "hidden_set.hpp"

#include <GarrysMod/InterfacePointers.hpp>
#include <GarrysMod/FactoryLoader.hpp>

#include <detouring/classproxy.hpp>

#include <tier0/dbg.h>
#include <eiface.h>
#include <edict.h>
#include <iservernetworkable.h>

namespace Vanish
{
	namespace CheckTransmit
	{
		static IVEngineServer *engine_server = nullptr;

		///@return entity index of the receiving client, 0 when unresolved
		static int32_t ReceiverOf( CCheckTransmitInfo *info )
		{
			if( info == nullptr || info->m_pClientEnt == nullptr || engine_server == nullptr )
				return 0;

			const int32_t index = engine_server->IndexOfEdict( info->m_pClientEnt );
			return HiddenSet::IsViewerIndex( index ) ? index : 0;
		}

		/// Removes every hidden entity from this client's transmit sets.
		static void StripHidden( CCheckTransmitInfo *info, int32_t receiver )
		{
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

		class Proxy : public Detouring::ClassProxy<IServerGameEnts, Proxy>
		{
		private:
			typedef IServerGameEnts TargetClass;
			typedef Proxy SubstituteClass;

		public:
			virtual void CheckTransmit(
				CCheckTransmitInfo *info,
				const unsigned short *edict_indices,
				int edict_count
			)
			{
				Call( &TargetClass::CheckTransmit, info, edict_indices, edict_count );

				const int32_t receiver = ReceiverOf( info );

				// a client that may see hidden players needs no filtering, and
				// an unresolved receiver is left alone rather than guessed at
				if( receiver == 0 || Hidden( ).IsExempt( receiver ) )
					return;

				StripHidden( info, receiver );
			}

			static Proxy Singleton;
		};

		Proxy Proxy::Singleton;

		bool Attach( )
		{
			engine_server = InterfacePointers::VEngineServer( );

			SourceSDK::FactoryLoader server_loader( "server" );
			IServerGameEnts *ents =
				server_loader.GetInterface<IServerGameEnts>( INTERFACEVERSION_SERVERGAMEENTS );

			if( engine_server == nullptr || ents == nullptr )
			{
				Warning( "[vanish] no IServerGameEnts, entity hiding is disabled\n" );
				return false;
			}

			return Proxy::Initialize( ents, &Proxy::Singleton )
				&& Proxy::Hook( &IServerGameEnts::CheckTransmit, &Proxy::CheckTransmit );
		}

		void Detach( )
		{
			Proxy::UnHook( &IServerGameEnts::CheckTransmit );
		}
	}
}
