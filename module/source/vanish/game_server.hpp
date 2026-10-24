#pragma once

class IServer;

namespace Vanish
{
	/// The engine's game server.
	///
	/// garrysmod_common finds it through the exported sv symbol, which the
	/// 32-bit Linux engine is stripped of; its fallback signature does not
	/// match there either. RTTI survives the strip, so that build finds the
	/// one CGameServer through it instead.
	namespace GameServer
	{
		///@return the game server, nullptr when it cannot be found
		IServer *Get( );
	}
}
