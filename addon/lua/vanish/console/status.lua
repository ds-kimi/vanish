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
---@return string the engine's "9s", "3m 9s" or "2h 3m"
local function uptime(seconds)
	local total = math.max(math.floor(seconds), 0)
	local minutes = math.floor(total / 60)

	if minutes >= 60 then
		return string.format("%ih %im", math.floor(minutes / 60), minutes % 60)
	elseif minutes > 0 then
		return string.format("%im %is", minutes, total % 60)
	end

	return string.format("%is", total)
end

-- column layout of the engine's table: userid right aligned, then name,
-- uniqueid, connected, ping, loss and state, each starting where the header
-- names it. Bots leave connected, ping and loss blank.
local ROW = "# %6i %-19s %-19s %-9s %4s %4s active\n"

---@param ply Player
---@return string row matching the engine column layout
local function statusRow(ply)
	-- quoted as is: the engine does not escape quotes inside a name
	local name = '"' .. ply:Nick() .. '"'

	if ply:IsBot() then
		return string.format(ROW, ply:UserID(), name, "BOT", "", "", "")
	end

	local connected = math.max(math.floor(ply:TimeConnected()), 0)

	return string.format(ROW, ply:UserID(), name, ply:SteamID(),
		string.format("%02i:%02i", math.floor(connected / 60), connected % 60),
		ply:Ping(), ply:PacketLoss())
end

--- Sends a status listing that only contains visible players.
---@param ply Player requester
function Vanish.SendStatus(ply)
	-- player.GetAll is masked, so vanished players cannot reach this listing
	local visible = player.GetAll()
	local humans = #player.GetHumans()
	local info = Vanish.ServerInfo()

	Vanish.SendLine(ply, string.format("hostname: %s\n", GetHostName()))
	Vanish.SendLine(ply, string.format("version : %s\n", info.version))
	Vanish.SendLine(ply, string.format("udp/ip  : %s%s\n", info.address,
		info.publicAddress and string.format("  (public ip: %s)", info.publicAddress) or ""))

	if info.steamID then
		Vanish.SendLine(ply, string.format("steamid : %s\n", info.steamID))
	end

	Vanish.SendLine(ply, string.format("map     : %s at: 0 x, 0 y, 0 z\n", game.GetMap()))
	Vanish.SendLine(ply, string.format("uptime  : %s map, %s server\n",
		uptime(CurTime()), uptime(SysTime() - Vanish.StartTime)))
	Vanish.SendLine(ply, string.format("players : %i humans, %i bots (%i max)\n",
		humans, #visible - humans, game.MaxPlayers()))
	Vanish.SendLine(ply,
		"# userid name                uniqueid            connected ping loss state\n")

	for _, target in ipairs(visible) do
		Vanish.SendLine(ply, statusRow(target))
	end
end

Vanish.Replies["status"] = Vanish.SendStatus
