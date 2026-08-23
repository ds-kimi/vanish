-- Reads the header values the engine prints in status.

---@param id64 string game server SteamID as a decimal string
---@return string rendered SteamID3 form, matching the engine's status line
local function renderSteamID(id64)
	local value = tonumber(id64) or 0
	local account = value % 4294967296
	local instance = math.floor(value / 4294967296) % 1048576
	local kind = math.floor(value / 4503599627370496) % 16
	local universe = math.floor(value / 72057594037927936) % 256

	-- 4 is an anonymous game server account, 3 a persistent one; anything else
	-- is not something a server can log in as
	local letter = kind == 4 and "A" or kind == 3 and "G" or "I"

	return string.format("[%s-%i:%i(%i)] (%s)", letter, universe, account, instance, id64)
end

---@return string version line in the engine's "patch/protocol build secure" form
local function readVersion()
	local inf = file.Read("steam.inf", "GAME")
	if not inf then return "unknown" end

	local patch = string.match(inf, "PatchVersion=([%w%.]+)") or "unknown"
	local protocol = string.match(inf, "ProtocolVersion=(%d+)") or "0"
	local build = string.match(inf, "ClientVersion=(%d+)") or "0"

	-- the engine appends "secure" whenever VAC is active for the session
	return string.format("%s/%s %s secure", patch, protocol, build)
end

--- Reads the values the engine prints in status but Lua cannot reach.
---@return table info version, publicAddress and steamID strings
function Vanish.ServerInfo()
	local cfg = Vanish.Config.status
	if cfg.spoof then return cfg end

	local public = GetConVar("net_public_adr")
	local id64 = vanishinfo and vanishinfo.GetSteamID64()

	return {
		version = readVersion(),
		publicAddress = public and public:GetString() ~= "" and public:GetString()
			or cfg.publicAddress,
		steamID = id64 and renderSteamID(id64) or cfg.steamID,
	}
end
