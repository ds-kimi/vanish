-- Silences a vanished player. Sounds are networked by position, not by the
-- transmit state of the entity that made them, so footsteps, weapon fire and
-- physics impacts all survive the hiding and give the position away.

---@param ent Entity emitter of the sound
---@return boolean owned whether the sound belongs to a vanished player
local function belongsToVanished(ent)
	if not IsValid(ent) then return false end

	if ent:IsPlayer() then return Vanish.IsVanished(ent) end

	-- weapons, ragdolls and anything parented to the player emit under their
	-- own entity, so the owner decides
	local owner = ent:GetOwner()
	if IsValid(owner) and owner:IsPlayer() and Vanish.IsVanished(owner) then return true end

	local parent = ent:GetParent()

	return IsValid(parent) and parent:IsPlayer() and Vanish.IsVanished(parent)
end

hook.Add("EntityEmitSound", "vanish_silence", function(data)
	if not Vanish.Config.silence.sounds then return end

	if belongsToVanished(data.Entity) then return false end
end)

-- footsteps take a separate path that never reaches EntityEmitSound on every
-- build, so they are stopped at their own hook as well
hook.Add("PlayerFootstep", "vanish_silence_steps", function(ply)
	if not Vanish.Config.silence.sounds then return end

	if Vanish.IsVanished(ply) then return true end
end)
