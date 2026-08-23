#pragma once

namespace Vanish
{
	/// Clears the transmit bit of hidden entities after the engine has decided
	/// what each client should receive.
	///
	/// CBasePlayer edicts are marked FL_EDICT_ALWAYS, so the engine writes them
	/// to every snapshot and Entity:SetPreventTransmit never applies to them.
	/// Running after the original call is what makes hiding a player possible.
	namespace CheckTransmit
	{
		///@return true when the hook is installed
		bool Attach( );

		void Detach( );
	}
}
