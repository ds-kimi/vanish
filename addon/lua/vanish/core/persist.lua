-- Keeps vanish across a map change. Lua state dies with the map, so without
-- this a vanished player is silently visible again the moment the map reloads,
-- while every client was last told they had disconnected.

local FILE = "vanish_state.txt"

---@return table<string, boolean> stored SteamID64 set from the previous map
local function read()
	if not Vanish.Config.persist then return {} end

	local raw = file.Read(FILE, "DATA")
	local stored = raw and util.JSONToTable(raw)

	return istable(stored) and stored or {}
end

-- players carried over from the previous map who have not reconnected yet.
-- They are kept apart from Vanished so a save triggered by the first one to
-- rejoin cannot drop the others.
Vanish.Stored = Vanish.Stored or read()

--- Writes the vanish set the next map should restore.
function Vanish.SaveState()
	if not Vanish.Config.persist then return end

	local out = {}
	for id in pairs(Vanish.Stored) do out[id] = true end
	for id in pairs(Vanish.Vanished) do out[id] = true end

	-- a player who is here and not vanished has unvanished deliberately, so
	-- their carried over entry must not outlive that decision
	for _, ply in ipairs(Vanish.RealGetAll()) do
		local id = ply:SteamID64()
		if id and not Vanish.Vanished[id] then out[id] = nil end
	end

	file.Write(FILE, util.TableToJSON(out))
end

-- authing happens before the first spawn, which is the last moment a player
-- can be hidden without ever having been transmitted to anybody
hook.Add("PlayerAuthed", "vanish_persist_restore", function(ply, steamid64)
	if not Vanish.Config.persist or not steamid64 then return end
	if not Vanish.Stored[steamid64] then return end

	Vanish.Stored[steamid64] = nil

	timer.Simple(0, function()
		-- restored only for players still allowed to use vanish, so a revoked
		-- SteamID cannot come back hidden
		if IsValid(ply) and Vanish.IsAllowed(ply) then
			Vanish.SetVanished(ply, true, true)
		end
	end)
end)

hook.Add("ShutDown", "vanish_persist_save", function() Vanish.SaveState() end)
