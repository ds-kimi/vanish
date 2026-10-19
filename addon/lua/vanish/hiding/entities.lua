-- Hides the player entity and everything it owns from other clients.
--
-- Only the module can do this. CBasePlayer edicts are FL_EDICT_ALWAYS, so
-- Entity:SetPreventTransmit never applies to the player itself, and hiding
-- only their weapons would leave a visible player holding nothing. Without
-- the module, hideEntities does nothing.

---@param ply Player
---@return Entity[] owned entities that must disappear together with the player
local function collectOwned(ply)
	local list = ply:GetChildren()

	for _, wep in ipairs(ply:GetWeapons()) do
		list[#list + 1] = wep
	end

	local ragdoll = ply:GetRagdollEntity()
	if IsValid(ragdoll) then
		list[#list + 1] = ragdoll
	end

	return list
end

---@param ply Player vanished player whose entities join the hidden set
local function hideOwner(ply)
	transmitcontrol.SetHidden(ply:EntIndex(), true)

	for _, ent in ipairs(collectOwned(ply)) do
		if IsValid(ent) then transmitcontrol.SetHidden(ent:EntIndex(), true) end
	end
end

--- Rebuilds the module's hidden set from the current vanish state.
function Vanish.RefreshTransmit()
	if not Vanish.Config.hideEntities or not transmitcontrol then return end

	-- rebuilt from scratch: entity indices are reused, so nothing from the
	-- previous pass may survive into this one
	transmitcontrol.Reset()

	if not Vanish.IsActive() then return end

	for _, ply in ipairs(Vanish.RealGetAll()) do
		local vanished = Vanish.IsVanished(ply)

		-- staff see vanished players, and so does every vanished player:
		-- the hook only spares a receiver its own player entity, and their
		-- weapons and children have to reach them too
		transmitcontrol.SetExempt(ply:EntIndex(), vanished or Vanish.IsAllowed(ply))

		if vanished then hideOwner(ply) end

		if Vanish.Config.debug then
			print(string.format("[vanish] idx=%i %s hidden=%s exempt=%s",
				ply:EntIndex(), ply:Nick(), tostring(vanished),
				tostring(vanished or Vanish.IsAllowed(ply))))
		end
	end
end

if Vanish.Config.debug then
	print(string.format("[vanish] transmitcontrol=%s", tostring(transmitcontrol ~= nil)))
end

local function refreshIfActive()
	if Vanish.IsActive() then Vanish.RefreshTransmit() end
end

-- a joining viewer needs their exemption set, and a respawn hands out new
-- weapons under new entity indices
hook.Add("PlayerInitialSpawn", "vanish_transmit_join", refreshIfActive)
hook.Add("PlayerSpawn", "vanish_transmit_spawn", refreshIfActive)

timer.Create("vanish_transmit_refresh", Vanish.Config.refreshInterval, 0, refreshIfActive)
