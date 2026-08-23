-- The two hook chains the fake drives, named so the callers read as what they
-- announce rather than as a hook name.

--- Makes admin mods log the departure of a player that stays connected.
---@param ply Player
function Vanish.RunDisconnectHooks(ply)
	Vanish.RunFakeHooks("PlayerDisconnected", ply)
end

--- Makes admin mods log the arrival of a player that never left, so a fake
--- disconnect is not left dangling in their logs.
---@param ply Player
function Vanish.RunJoinHooks(ply)
	if not Vanish.Config.disconnect.joinHooks then return end

	-- both chains run because mods split join logging between them: the
	-- authed pass has the SteamID, the spawn pass has the player entity
	Vanish.RunFakeHooks("PlayerAuthed", ply)
	Vanish.RunFakeHooks("PlayerInitialSpawn", ply)
end
