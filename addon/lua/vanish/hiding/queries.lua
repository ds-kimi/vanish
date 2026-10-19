-- Hides vanished players from A2S_INFO and A2S_PLAYER replies.

local enabled = false

--- Feeds the query cache with the roster external tools are allowed to see.
local function pushPlayers()
	-- player.GetAll is already masked, so vanished players never enter the list
	local visible = player.GetAll()

	vanishquery.ResetPlayers()
	vanishquery.SetPlayerCount(#visible)

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

	-- while nobody is vanished the engine answers queries itself, which keeps
	-- the reply correct without this module having to mirror every field
	if active ~= enabled then
		vanishquery.SetEnabled(active)
		enabled = active
	end

	if active then pushPlayers() end
end

if not vanishquery and Vanish.Config.hideFromQueries then
	ErrorNoHalt("[vanish] query filtering unavailable, A2S replies are unfiltered\n")
end

timer.Create("vanish_query_refresh", Vanish.Config.refreshInterval, 0, Vanish.RefreshQuery)
