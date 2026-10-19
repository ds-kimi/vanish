-- The filtered stats reply, built from the masked roster.
--
-- Only the player count would give a vanished player away, but the engine
-- prints the whole row at once, so the row is rebuilt. Everything except the
-- count comes from the engine itself through the module, so the numbers are
-- real rather than plausible.

-- header and column widths are the engine's own, taken from engine.dll and
-- checked against real output
local HEADER =
	"CPU    In_(KB/s)  Out_(KB/s)  Uptime  Map_changes  FPS      Players  Connects\n"
local ROW = "%-7.2f%-11.2f%-12.2f%-8i%-13i%-9.2f%-9i%-8i\n"

---@type number connections seen since this Lua state loaded
local connects = 0

---@return number cpu, number inRate, number outRate, number mapChanges
local function engineStats()
	if not vanishinfo then return 0, 0, 0, 0 end

	local inRate, outRate = vanishinfo.GetNetStats()
	local spawns = vanishinfo.GetSpawnCount()

	-- the engine counts map changes, and the spawn count includes the first
	-- level load, so one is taken off it
	return vanishinfo.GetCPUUsage() or 0, inRate or 0, outRate or 0,
		math.max((spawns or 1) - 1, 0)
end

--- Sends a stats row whose player count matches the masked roster.
---@param ply Player requester
function Vanish.SendStats(ply)
	local cpu, inRate, outRate, mapChanges = engineStats()

	Vanish.SendLine(ply, HEADER)
	Vanish.SendLine(ply, string.format(ROW,
		cpu, inRate, outRate,
		math.floor(SysTime() / 60),
		mapChanges,
		1 / math.max(FrameTime(), 0.001),
		#player.GetAll(),
		connects))
end

-- PlayerConnect, not PlayerAuthed: the engine counts bots, and bots are
-- never authed
hook.Add("PlayerConnect", "vanish_stats_connects", function()
	connects = connects + 1
end)

Vanish.Replies["stats"] = Vanish.SendStats
