-- Keeps the vanished protections in place across respawns and damage.

-- a respawn resets movetype, godmode and collision group, so the state is
-- reapplied once the gamemode has finished setting the player up. The state
-- captured at vanish time is kept: capturing again here would record the
-- protections themselves, notarget included, as what unvanishing restores
hook.Add("PlayerSpawn", "vanish_protect_spawn", function(ply)
	if not Vanish.IsVanished(ply) then return end

	timer.Simple(0, function()
		if IsValid(ply) and Vanish.IsVanished(ply) then
			Vanish.ApplyProtection(ply, true)
		end
	end)
end)

-- gamemodes that damage players without consulting FL_GODMODE, and anything
-- calling TakeDamage directly, are stopped here instead
hook.Add("EntityTakeDamage", "vanish_protect_damage", function(target)
	if not Vanish.Config.protect or not Vanish.Config.protection.godmode then return end

	if Vanish.IsVanished(target) then return true end
end)
