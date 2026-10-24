-- Restores the vanish state persist.lua carried over from the previous map.

---@param ply Player
local function restore(ply)
	if not Vanish.Config.persist or not IsValid(ply) then return end

	-- PlayerAuthed passes the STEAM_0 form, the store is keyed by SteamID64,
	-- and an unauthed player has none yet
	local steamid64 = ply:SteamID64()
	if not steamid64 or not Vanish.Stored[steamid64] then return end

	-- an unvalidated SteamID is only a claim: the entry waits for PlayerAuthed
	-- instead of being consumed by a spawn that cannot be trusted yet
	if Vanish.Config.requireSteamAuth and not ply:IsFullyAuthenticated() then return end

	Vanish.Stored[steamid64] = nil

	timer.Simple(0, function()
		-- restored only for players still allowed to use vanish, so a revoked
		-- SteamID cannot come back hidden
		if IsValid(ply) and Vanish.IsAllowed(ply) then
			Vanish.SetVanished(ply, true, true)
		end
	end)
end

-- authing normally happens before the first spawn, the last moment a player
-- can be hidden without ever having been transmitted to anybody. A slow auth
-- lands after it, so whichever comes first restores and the other finds the
-- entry gone
hook.Add("PlayerAuthed", "vanish_persist_restore", restore)
hook.Add("PlayerInitialSpawn", "vanish_persist_restore", restore)

hook.Add("ShutDown", "vanish_persist_save", function() Vanish.SaveState() end)
