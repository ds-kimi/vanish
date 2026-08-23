-- Vanish state: who may toggle it and who is currently hidden.

---@param ply Player
---@return boolean allowed whether this player may use the vanish command
function Vanish.IsAllowed(ply)
	if not IsValid(ply) or not ply:IsPlayer() then return false end

	return Vanish.Config.allowed[ply:SteamID()] == true
end

---@param ply Player
---@return boolean vanished
function Vanish.IsVanished(ply)
	if not IsValid(ply) or not ply:IsPlayer() then return false end

	return Vanish.Vanished[ply:SteamID64() or ""] == true
end

---@return boolean active whether at least one player is currently vanished
function Vanish.IsActive()
	return next(Vanish.Vanished) ~= nil
end

---@param ply Player
---@param state boolean
function Vanish.SetVanished(ply, state)
	if not IsValid(ply) or not ply:IsPlayer() then return end

	local id = ply:SteamID64()
	if not id then return end

	Vanish.Vanished[id] = state or nil

	-- transmit flags are per-viewer and survive nothing, so they are rebuilt
	-- immediately instead of waiting for the periodic refresh
	Vanish.RefreshTransmit()
	Vanish.RefreshQuery()

	Vanish.ApplyProtection(ply, state)

	-- the announcement goes out after the roster is already masked, so any
	-- handler it reaches sees the same player list clients now have
	if state then
		Vanish.FakeDisconnect(ply)
	else
		Vanish.FakeRejoin(ply)
	end
end
