Vanish = Vanish or {}

---@type table<string, boolean> SteamID64 -> vanished
Vanish.Vanished = Vanish.Vanished or {}

-- used for the uptime line of the synthesised status reply
Vanish.StartTime = Vanish.StartTime or SysTime()

--[[--------------------------------------------------------------------------
	Basic settings
----------------------------------------------------------------------------]]

---@class VanishConfig
Vanish.Config = {
	-- SteamIDs allowed to run the vanish command. Nobody else can toggle it,
	-- and these players keep seeing vanished players themselves.
	allowed = {
		["STEAM_0:0:440944583"] = true,
	},

	-- hide the player entity from other clients, so a vanished player leaves
	-- the world and every scoreboard. Needs gmsv_vanish.
	hideEntities = true,

	-- answer status and ping with a filtered listing instead of letting the
	-- engine reply. Needs gmsv_sourcenet.
	filterCommands = true,

	-- hide vanished players from A2S_INFO and A2S_PLAYER, which covers the
	-- server browser and external query tools. Needs gmsv_vanish.
	-- WARNING: query replies that disagree with the real player list are what
	-- count spoofers do, and Facepunch delists servers for it. See the readme.
	hideFromQueries = true,

	-- broadcast a real player_disconnect game event on vanish, so every
	-- client prints the usual leave message and drops the player from its
	-- own bookkeeping exactly as a genuine disconnect would. Needs gmsv_vanish.
	fakeDisconnect = true,

	-- godmode, noclip, notarget and pass-through collisions while vanished.
	protect = true,

	-- keep the vanish across a map change, restored before the first spawn.
	persist = true,

	-- blank the vanished player's userinfo string table entry, the last copy
	-- of their name and SteamID left on every client. Needs gmsv_vanish.
	-- Side effect: the entry is global, so the vanished player's own client
	-- loses its name too, and a HUD printing LocalPlayer():Nick() draws blank
	-- for them. Off by default for that reason.
	hideUserinfo = false,

	-- server console tracing. Never sends anything to clients.
	debug = false,
}

--[[--------------------------------------------------------------------------
	Advanced settings
----------------------------------------------------------------------------]]

-- Client commands whose engine reply enumerates connected players. Every one
-- is dropped before the engine sees it; those with an entry in Vanish.Replies
-- (status, ping) are answered with a filtered version instead.
Vanish.Config.blockedCommands = {
	["status"] = true,
	["ping"] = true,
	["users"] = true,
	["stats"] = true,
}

-- Seconds between transmit refreshes. Weapons and children appear after a
-- spawn, so state is re-applied periodically instead of only on toggle.
Vanish.Config.refreshInterval = 2

-- The synthesised status reply reports the server's real values. Set spoof to
-- true to send the fixed strings below instead, for instance to hide a
-- development address from players.
Vanish.Config.status = {
	spoof = false,

	version = "2026.04.29/24 10029 secure",
	publicAddress = "12.34.567.89",
	steamID = "[A-1:0(0)] (0)",
}

-- How the fake disconnect is presented and which server side code it drives.
Vanish.Config.disconnect = {
	-- reason shown in the client leave message, engine formats it as
	-- "Name has left the game (reason)"
	reason = "Disconnect by user.",

	-- announce the return with a player_connect event on unvanish, so
	-- reappearing looks like a rejoin instead of a player materialising
	announceRejoin = true,

	-- server side PlayerDisconnected hooks to run so admin mods log the
	-- departure: "none", "match" (identifiers matching hookPatterns) or "all".
	-- "all" reaches gamemode logic too: DarkRP sells doors, drops money and
	-- saves data on disconnect, which is not undone when the player unvanishes.
	hooks = "match",

	-- lowercase substrings matched against hook identifiers under "match".
	-- The defaults reach the logging handlers, ULX's and FAdmin's included,
	-- and nothing that changes player state. Adding "sam" pulls in SAM's gag,
	-- jail, playtime and rank timer handlers, which log nothing and do act on
	-- a player that is still connected.
	hookPatterns = {"ulx", "ulib", "log"},

	-- run the same selection of PlayerAuthed and PlayerInitialSpawn handlers
	-- on unvanish, so a logged departure is followed by a logged return
	joinHooks = true,
}

-- What a vanished player stops emitting. Every one of these is networked by
-- position or by name rather than by the transmit state of the entity, so
-- hiding the entity alone does not stop any of them.
Vanish.Config.silence = {
	-- footsteps, weapon fire, physics impacts
	sounds = true,

	-- voice chat, staff still hear a vanished player
	voice = true,

	-- chat messages, echoed to staff and the speaker instead
	chat = true,

	-- damage they deal, which is what drives the kill feed and damage logs
	blockOutgoingDamage = true,

	-- suicide, which would broadcast a death and drop a named ragdoll
	blockSuicide = true,

	-- the attack buttons. Firing sends tracers, muzzle flashes and impact
	-- decals to everyone in range no matter who fired, so the effects are
	-- stopped at the input. Set false to keep the physgun and toolgun usable
	-- while vanished, and accept the effects they draw.
	blockAttacks = true,
}

-- What a vanished player is protected from. Every value is restored on
-- unvanish from the state captured when the player vanished.
Vanish.Config.protection = {
	godmode = true,
	noclip = true,

	-- NPCs and turrets stop acquiring the player as a target
	notarget = true,

	-- players and props stop colliding with the vanished player
	noCollide = true,
}
