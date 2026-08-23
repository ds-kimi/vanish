-- Keeps a vanished player out of combat in both directions. Damage they deal
-- is what the kill feed, the death events and every damage log react to, and
-- all of those name the player the transmit hiding just removed.

hook.Add("EntityTakeDamage", "vanish_combat_outgoing", function(_, dmginfo)
	if not Vanish.Config.silence.blockOutgoingDamage then return end

	local attacker = dmginfo:GetAttacker()
	if Vanish.IsVanished(attacker) then return true end

	-- a rocket or a thrown prop carries its owner rather than being one
	local inflictor = dmginfo:GetInflictor()
	if IsValid(inflictor) and Vanish.IsVanished(inflictor:GetOwner()) then return true end
end)

-- a suicide would broadcast a death event and drop a ragdoll under a name the
-- other clients were told had left
hook.Add("CanPlayerSuicide", "vanish_combat_suicide", function(ply)
	if not Vanish.Config.silence.blockSuicide then return end

	if Vanish.IsVanished(ply) then return false end
end)

-- nothing may target a vanished player either, which covers gamemodes that
-- pick victims from their own bookkeeping instead of a trace
hook.Add("PlayerShouldTakeDamage", "vanish_combat_incoming", function(ply)
	if not Vanish.Config.protect then return end

	if Vanish.IsVanished(ply) then return false end
end)

-- firing leaves tracers, muzzle flashes, impact effects and decals, all sent
-- as temp entities to everyone in range regardless of who fired them. Blocking
-- the attack buttons is what stops the effects, not blocking the damage.
hook.Add("StartCommand", "vanish_combat_attack", function(ply, cmd)
	if not Vanish.Config.silence.blockAttacks then return end
	if not Vanish.IsVanished(ply) then return end

	cmd:RemoveKey(IN_ATTACK)
	cmd:RemoveKey(IN_ATTACK2)
end)
