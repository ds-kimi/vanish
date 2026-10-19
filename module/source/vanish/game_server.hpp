#pragma once

class IServer;

namespace Vanish
{
	/// The engine's game server, which garrysmod_common cannot find on Linux.
	///
	/// Its lookup there wants an `sv` symbol the shipping engine is stripped
	/// of, and its fallback signature no longer matches. RTTI survives the
	/// strip, so the Linux build finds the one CGameServer through that.
	namespace GameServer
	{
		///@return the game server, nullptr when it cannot be found
		IServer *Get( );
	}
}
