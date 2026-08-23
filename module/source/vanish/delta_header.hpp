#pragma once

namespace Vanish
{
	/// Turns the leave-PVS header of a hidden entity into a delete.
	///
	/// The engine only sets FHDR_DELETE for entities that left the global frame
	/// snapshot. A per-client hide never does, so the client marks the entity
	/// dormant and keeps it, name and SteamID included, in its entity list.
	/// Promoting the header makes that one client destroy it instead.
	namespace DeltaHeader
	{
		///@param engine_module handle of the loaded engine binary
		///@return true when the detour is installed
		bool Attach( void *engine_module );

		void Detach( );
	}
}
