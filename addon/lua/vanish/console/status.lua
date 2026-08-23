-- The filtered status reply, built from the masked roster.

-- TextMsg carries console prints and the engine refuses anything over 255
-- bytes, so the listing is sent one bounded line at a time
local MAX_LINE = 200

---@type table<string, fun(ply:Player)> command name -> synthesised reply
Vanish.Replies = Vanish.Replies or {}

---@param ply Player
---@param text string single line, sent split when oversized
function Vanish.SendLine(ply, text)
	while #text > MAX_LINE do
		ply:PrintMessage(HUD_PRINTCONSOLE, string.sub(text, 1, MAX_LINE))
		text = string.sub(text, MAX_LINE + 1)
	end

	ply:PrintMessage(HUD_PRINTCONSOLE, text)
end

---@param seconds number
---@return string engine style "3m 9s"
local function uptime(seconds)
	local total = math.max(math.floor(seconds), 0)

	return string.format("%im %is", math.floor(total / 60), total % 60)
end

---@param ply Player
---@return string row matching the engine column layout
local function statusRow(ply)
	local connected = math.max(math.floor(ply:TimeConnected()), 0)

	return string.format("#%6i %-19s %-19s %-11s %3i %4i active\n",
		ply:UserID(),
		string.format("%q", ply:Nick()),
		ply:SteamID(),
		string.format("%02i:%02i", math.floor(connected / 60), connected % 60),
		ply:Ping(),
		ply:PacketLoss())
end

--- Sends a status listing that only contains visible players.
---@param ply Player requester
function Vanish.SendStatus(ply)
	-- player.GetAll is masked, so vanished players cannot reach this listing
	local visible = player.GetAll()
	local bots = #player.GetBots()
	local info = Vanish.ServerInfo()

	Vanish.SendLine(ply, string.format("hostname: %s\n", GetHostName()))
	Vanish.SendLine(ply, string.format("version : %s\n", info.version))
	Vanish.SendLine(ply, string.format("udp/ip  : %s  (public ip: %s)\n",
		game.GetIPAddress(), info.publicAddress))
	Vanish.SendLine(ply, string.format("steamid : %s\n", info.steamID))
	Vanish.SendLine(ply, string.format("map     : %s at: 0 x, 0 y, 0 z\n", game.GetMap()))
	Vanish.SendLine(ply, string.format("uptime  : %s map, %s server\n",
		uptime(CurTime()), uptime(SysTime() - Vanish.StartTime)))
	Vanish.SendLine(ply, string.format("players : %i humans, %i bots (%i max)\n",
		#visible - bots, bots, game.MaxPlayers()))
	Vanish.SendLine(ply,
		"# userid name                uniqueid            connected ping loss state\n")

	for _, target in ipairs(visible) do
		Vanish.SendLine(ply, statusRow(target))
	end
end

Vanish.Replies["status"] = Vanish.SendStatus
