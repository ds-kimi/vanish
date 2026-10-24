-- The vanish console command.

---@type table<string, number> SteamID64 -> CurTime threshold
local nextAllowed = {}

---@param ply Player
---@return boolean limited whether the player toggled too recently
local function isRateLimited(ply)
	local id = ply:SteamID64()
	if not id then return true end

	if (nextAllowed[id] or 0) > CurTime() then return true end

	nextAllowed[id] = CurTime() + 1
	return false
end

---@param ply Player
local function toggle(ply)
	local state = not Vanish.IsVanished(ply)
	Vanish.SetVanished(ply, state)

	ply:PrintMessage(HUD_PRINTCONSOLE,
		state and "vanish: on\n" or "vanish: off\n")
end

concommand.Add("vanish", function(ply)
	-- unauthorised callers get no output at all: any reply would tell an
	-- attacker the command exists on this server
	if not Vanish.IsAllowed(ply) then return end
	if isRateLimited(ply) then return end

	toggle(ply)
end)

hook.Add("PlayerDisconnected", "vanish_cleanup", function(ply)
	-- the fake disconnect drives this same hook, and unvanishing the player
	-- there would undo the vanish it is announcing
	if Vanish.Faking then return end

	if not IsValid(ply) then return end

	local id = ply:SteamID64()
	if not id then return end

	-- a real disconnect while vanished is a departure, not a decision to be
	-- visible again, so the state is carried to the next session
	if Vanish.Config.persist and Vanish.Vanished[id] then
		Vanish.Stored[id] = true
	end

	Vanish.MarkVanished(ply, id, false)
	nextAllowed[id] = nil

	-- the player is still listed during this hook, and must not count as
	-- one who stayed and unvanished
	Vanish.SaveState(ply)

	-- entity indices are reused, so stale hidden slots must not outlive the
	-- player that owned them
	timer.Simple(0, Vanish.RefreshTransmit)
end)
