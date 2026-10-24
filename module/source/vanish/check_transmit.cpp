#include "check_transmit.hpp"
#include "hidden_set.hpp"
#include "transmit_strip.hpp"

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

				const int32_t receiver = TransmitStrip::ReceiverOf( engine_server, info );

				// a client that may see hidden players needs no filtering, and
				// an unresolved receiver is left alone rather than guessed at
				if( receiver == 0 || Hidden( ).IsExempt( receiver ) )
					return;

				TransmitStrip::StripHidden( info, receiver );
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
