-- Vanish state: who may toggle it and who is currently hidden.

-- Mirrors Vanish.Vanished keyed by the player entity. IsVanished runs for every
-- player in every masked library call, many times a tick on a full server, and
-- a table lookup by entity avoids building a SteamID64 string each time. Weak
-- keys let a removed entity drop out even if a cleanup hook never ran.
---@type table<Player, boolean>
Vanish.VanishedEnts = Vanish.VanishedEnts or setmetatable({}, {__mode = "k"})

---@param ply Player
---@return boolean allowed whether this player may use the vanish command
function Vanish.IsAllowed(ply)
	if not IsValid(ply) or not ply:IsPlayer() then return false end

	-- until Steam has validated the ticket, SteamID is only what the client
	-- claimed, and a staff ID would otherwise be honoured for that window
	if Vanish.Config.requireSteamAuth and not ply:IsFullyAuthenticated() then return false end

	return Vanish.Config.allowed[ply:SteamID()] == true
end

---@param ply Player
---@return boolean vanished
function Vanish.IsVanished(ply)
	return Vanish.VanishedEnts[ply] == true
end

---@return boolean active whether at least one player is currently vanished
function Vanish.IsActive()
	return next(Vanish.Vanished) ~= nil
end

--- Records the state in both tables, the persistent one by SteamID64 and
--- the fast one by entity.
---@param ply Player
---@param id string SteamID64
---@param state boolean
function Vanish.MarkVanished(ply, id, state)
	Vanish.Vanished[id] = state or nil
	Vanish.VanishedEnts[ply] = state or nil
end
