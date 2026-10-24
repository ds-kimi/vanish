-- Closes the window the periodic refresh leaves open. A weapon picked up, a
-- prop spawned or a ragdoll created after the last refresh is transmitted to
-- everybody until the timer comes round, which is up to refreshInterval
-- seconds of a vanished player's belongings being visible.

--- Rebuilds transmit state on the next frame, collapsing a burst of entity
--- creations into a single pass.
local function refreshSoon()
	if not Vanish.IsActive() then return end

	-- a named single shot timer restarts instead of stacking, so spawning
	-- fifty props costs one refresh rather than fifty
	timer.Create("vanish_transmit_soon", 0, 1, Vanish.RefreshTransmit)
end

---@param wep Weapon
---@param owner Player
local function hideWeaponNow(wep, owner)
	-- the deferred refresh runs a frame late, and the snapshot sent in between
	-- would carry a weapon whose owner is the vanished player's index
	if transmitcontrol and Vanish.Config.hideEntities and Vanish.IsVanished(owner) and IsValid(wep) then
		transmitcontrol.SetHidden(wep:EntIndex(), true)
	end

	refreshSoon()
end

hook.Add("OnEntityCreated", "vanish_transmit_created", refreshSoon)
hook.Add("WeaponEquip", "vanish_transmit_weapon", hideWeaponNow)
hook.Add("PlayerSpawnedProp", "vanish_transmit_prop", refreshSoon)
hook.Add("PlayerSpawnedRagdoll", "vanish_transmit_ragdoll", refreshSoon)
hook.Add("CreateEntityRagdoll", "vanish_transmit_deathragdoll", refreshSoon)
