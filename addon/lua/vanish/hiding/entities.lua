-- Hides the player entity and everything it owns from other clients.

---@param ply Player
---@return Entity[] owned entities that must disappear together with the player
local function collectOwned(ply)
	local list = {}

	for _, ent in ipairs(ply:GetChildren()) do
		list[#list + 1] = ent
	end

	for _, wep in ipairs(ply:GetWeapons()) do
		list[#list + 1] = wep
	end

	local ragdoll = ply:GetRagdollEntity()
	if IsValid(ragdoll) then
		list[#list + 1] = ragdoll
	end

	return list
end

---@param ply Player owner whose entities get their transmit state rewritten
---@param viewers Player[] every real player on the server
local function applyOwner(ply, viewers)
	local hidden = Vanish.IsVanished(ply)

	for _, ent in ipairs(collectOwned(ply)) do
		if IsValid(ent) then
			-- owned entities are ordinary edicts, so prevent transmit stops
			-- them being sent, while the module promotes the resulting
			-- leave-PVS into a delete so nothing stays dormant client side
			if Vanish.Config.hideEntities and transmitcontrol then
				transmitcontrol.SetHidden(ent:EntIndex(), hidden)
			end

			for _, viewer in ipairs(viewers) do
				-- staff keep seeing each other, otherwise two vanished admins
				-- would be unable to work together
				ent:SetPreventTransmit(viewer,
					hidden and viewer ~= ply and not Vanish.IsAllowed(viewer))
			end
		end
	end
end

--- Rebuilds transmit state for every player against every viewer.
function Vanish.RefreshTransmit()
	local viewers = Vanish.RealGetAll()

	local control = Vanish.Config.hideEntities and transmitcontrol or nil

	if control then control.Reset() end

	for _, ply in ipairs(viewers) do
		applyOwner(ply, viewers)

		-- CBasePlayer edicts are FL_EDICT_ALWAYS, so their transmit bit is
		-- cleared by the CheckTransmit hook rather than by prevent transmit
		if control then
			control.SetHidden(ply:EntIndex(), Vanish.IsVanished(ply))
			control.SetExempt(ply:EntIndex(), Vanish.IsAllowed(ply))

			if Vanish.Config.debug then
				print(string.format("[vanish] idx=%i %s hidden=%s exempt=%s",
					ply:EntIndex(), ply:Nick(), tostring(Vanish.IsVanished(ply)),
					tostring(Vanish.IsAllowed(ply))))
			end
		end
	end
end

if Vanish.Config.debug then
	print(string.format("[vanish] transmitcontrol=%s", tostring(transmitcontrol ~= nil)))
end

hook.Add("PlayerInitialSpawn", "vanish_transmit_join", Vanish.RefreshTransmit)
hook.Add("PlayerSpawn", "vanish_transmit_spawn", Vanish.RefreshTransmit)

timer.Create("vanish_transmit_refresh", Vanish.Config.refreshInterval, 0, function()
	if not Vanish.IsActive() then return end

	Vanish.RefreshTransmit()
end)
