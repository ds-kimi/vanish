-- Godmode, noclip and collision handling for a vanished player. Every one of
-- these is an engine level flag reachable from Lua, so nothing here needs the
-- module: GodEnable sets FL_GODMODE, which CBasePlayer checks before damage,
-- and SetMoveType writes MOVETYPE_NOCLIP the same way the cheat does.

-- keyed by the player entity: bots have no SteamID64, and the state only has
-- to live as long as the connection
---@type table<Player, table> state captured when vanishing
local saved = {}

---@param ply Player
local function capture(ply)
	if saved[ply] then return end

	saved[ply] = {
		moveType = ply:GetMoveType(),
		collisionGroup = ply:GetCollisionGroup(),
		godmode = ply:HasGodMode(),
		-- there is no GetNoTarget, the flag SetNoTarget writes is read instead
		noTarget = ply:IsFlagSet(FL_NOTARGET),
	}
end

---@param ply Player
local function restore(ply)
	local state = saved[ply]
	if not state then return end

	saved[ply] = nil

	ply:SetMoveType(state.moveType)
	ply:SetCollisionGroup(state.collisionGroup)
	ply:SetNoTarget(state.noTarget)

	if state.godmode then ply:GodEnable() else ply:GodDisable() end
end

--- Applies or lifts the vanished protections for one player.
---@param ply Player
---@param state boolean whether the player is now vanished
function Vanish.ApplyProtection(ply, state)
	if not Vanish.Config.protect then return end
	if not IsValid(ply) or not ply:IsPlayer() then return end

	if not state then return restore(ply) end

	capture(ply)

	local cfg = Vanish.Config.protection

	if cfg.godmode then ply:GodEnable() end
	if cfg.noclip then ply:SetMoveType(MOVETYPE_NOCLIP) end
	if cfg.notarget then ply:SetNoTarget(true) end

	-- COLLISION_GROUP_WORLD is what the engine uses for things players and
	-- props are expected to move through
	if cfg.noCollide then ply:SetCollisionGroup(COLLISION_GROUP_WORLD) end
end

hook.Add("PlayerDisconnected", "vanish_protect_cleanup", function(ply)
	if Vanish.Faking then return end

	saved[ply] = nil
end)
