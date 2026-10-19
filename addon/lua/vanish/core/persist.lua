-- Keeps vanish across a map change. Lua state dies with the map, so without
-- this a vanished player is silently visible again the moment the map reloads,
-- while every client was last told they had disconnected.

local FILE = "vanish_state.txt"

-- One SteamID64 per line rather than JSON: util.JSONToTable turns numeric
-- keys into numbers, and a SteamID64 does not survive the trip through a
-- double, so a restored key never matched anybody.

---@return table<string, boolean> stored SteamID64 set from the previous map
local function read()
	if not Vanish.Config.persist then return {} end

	local stored = {}
	for id in string.gmatch(file.Read(FILE, "DATA") or "", "%d+") do
		stored[id] = true
	end

	return stored
end

-- players carried over from the previous map who have not reconnected yet.
-- They are kept apart from Vanished so a save triggered by the first one to
-- rejoin cannot drop the others.
Vanish.Stored = Vanish.Stored or read()

--- Writes the vanish set the next map should restore.
---@param leaving Player|nil a player disconnecting right now, still listed
function Vanish.SaveState(leaving)
	if not Vanish.Config.persist then return end

	local out = {}
	for id in pairs(Vanish.Stored) do out[id] = true end
	for id in pairs(Vanish.Vanished) do out[id] = true end

	-- a player who is here and not vanished has unvanished deliberately, so
	-- their carried over entry must not outlive that decision
	for _, ply in ipairs(Vanish.RealGetAll()) do
		local id = ply ~= leaving and ply:SteamID64()
		if id and not Vanish.Vanished[id] then out[id] = nil end
	end

	file.Write(FILE, table.concat(table.GetKeys(out), "\n"))
end

---@param ply Player
local function restore(ply)
	if not Vanish.Config.persist or not IsValid(ply) then return end

	-- PlayerAuthed passes the STEAM_0 form, the store is keyed by SteamID64,
	-- and an unauthed player has none yet
	local steamid64 = ply:SteamID64()
	if not steamid64 or not Vanish.Stored[steamid64] then return end

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
