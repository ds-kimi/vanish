-- Hides vanished players from A2S_INFO and A2S_PLAYER replies.

local enabled = false

-- the engine convars the replies follow, read here because the module cannot
-- reach the engine's convar system on every platform
local CONVARS = {
	"sv_visiblemaxplayers", "sv_location",
	"sv_max_queries_sec", "sv_max_queries_window", "sv_max_queries_sec_global",
}

local function pushConVars()
	local values = {}
	for _, name in ipairs(CONVARS) do
		local cvar = GetConVar(name)
		if cvar then
			values[name] = name == "sv_location" and cvar:GetString() or cvar:GetFloat()
		end
	end

	vanishquery.SetConVars(values)
end

--- Feeds the query cache with the roster external tools are allowed to see.
local function pushPlayers()
	-- player.GetAll is already masked, so vanished players never enter the list
	local visible = player.GetAll()

	local bots = 0
	for _, ply in ipairs(visible) do
		if ply:IsBot() then bots = bots + 1 end
	end

	vanishquery.ResetPlayers()
	vanishquery.SetPlayerCount(#visible, bots)

	-- the engine reports the session's real VAC state, so the built reply
	-- does too. Only an older module without IsSecure falls back to asserting
	-- secure, the state almost every public server is in
	local secure = vanishinfo and vanishinfo.IsSecure and vanishinfo.IsSecure()
	vanishquery.SetSecure(secure ~= false)

	for _, ply in ipairs(visible) do
		vanishquery.AddPlayer(ply:Nick(), ply:Frags(), ply:TimeConnected())
	end
end

--- Matches query spoofing to the current vanish state.
function Vanish.RefreshQuery()
	if not vanishquery or not Vanish.Config.hideFromQueries then return end

	local active = Vanish.IsActive()

	-- filled before replies switch on, so the first one already carries the
	-- masked roster rather than whatever the last session left behind
	if active then
		pushConVars()
		pushPlayers()
	end

	-- while nobody is vanished the engine answers queries itself, which keeps
	-- the reply correct without this module having to mirror every field
	if active ~= enabled then
		vanishquery.SetEnabled(active)
		enabled = active
	end
end

if not vanishquery and Vanish.Config.hideFromQueries then
	ErrorNoHalt("[vanish] query filtering unavailable, A2S replies are unfiltered\n")
end

timer.Create("vanish_query_refresh", Vanish.Config.refreshInterval, 0, Vanish.RefreshQuery)
