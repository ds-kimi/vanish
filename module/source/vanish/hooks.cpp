#include "hooks.hpp"
#include "check_transmit.hpp"
#include "delta_header.hpp"

#include "../main.hpp"

#include <tier0/dbg.h>

#if defined SYSTEM_WINDOWS
#include <Windows.h>
#endif

namespace Vanish
{
	namespace Hooks
	{
		static bool transmit_hooked = false;
		static bool delete_hooked = false;

		///@return handle of the loaded engine binary, nullptr when unavailable
		static void *EngineModule( )
		{
			void *module = global::engine_loader.GetModule( );

#if defined SYSTEM_WINDOWS

			// resolving against the loaded handle avoids the name mismatch that
			// makes a path based lookup fail for the engine
			if( module == nullptr )
				module = GetModuleHandleA( "engine.dll" );

#endif

			return module;
		}

		void Attach( )
		{
			transmit_hooked = CheckTransmit::Attach( );
			if( !transmit_hooked )
			{
				Warning( "[vanish] CheckTransmit hook failed, entity hiding is disabled\n" );
				return;
			}

			delete_hooked = DeltaHeader::Attach( EngineModule( ) );

			Msg( delete_hooked
				? "[vanish] ready\n"
				: "[vanish] ready without client side deletes\n" );
		}

		void Detach( )
		{
			if( delete_hooked )
			{
				DeltaHeader::Detach( );
				delete_hooked = false;
			}

			if( transmit_hooked )
			{
				CheckTransmit::Detach( );
				transmit_hooked = false;
			}
		}

		bool Ready( )
		{
			return transmit_hooked && delete_hooked;
		}
	}
}
