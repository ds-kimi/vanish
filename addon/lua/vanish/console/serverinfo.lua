-- Reads the header values the engine prints in status.

-- steam.inf does not change while the server runs, and a dedicated server's
-- copy carries PatchVersion and nothing else
local PATCH = string.match(file.Read("steam.inf", "GAME") or "", "PatchVersion=([%w%.]+)") or "unknown"

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

---@return string version in the engine's "patch/protocol build secure" form
local function readVersion()
	local cfg = Vanish.Config.status

	-- the engine prints its network protocol as a constant, and the build
	-- through build_number( ), which only the module can reach
	local build = vanishinfo and vanishinfo.GetServerVersion() or cfg.build

	-- the engine prints "secure" or "insecure" from the session's VAC state;
	-- an older module that cannot say is assumed secure
	local secure = vanishinfo and vanishinfo.IsSecure and vanishinfo.IsSecure()

	return string.format("%s/%i %i %s", PATCH, cfg.protocol, build,
		secure == false and "insecure" or "secure")
end

---@return string address the address the engine reports itself bound to
local function boundAddress()
	local ip = GetConVar("ip"):GetString()

	-- an unset ip makes the engine bind to its resolved host address, which
	-- game.GetIPAddress reports as well
	if ip == "" or ip == "localhost" then
		ip = string.match(game.GetIPAddress(), "^[^:]+") or ip
	end

	return string.format("%s:%i", ip, GetConVar("hostport"):GetInt())
end

--- Reads the values the engine prints in status but Lua cannot reach. Each
--- field is nil where the engine prints nothing for it.
---@return table info version, address, publicAddress and steamID strings
function Vanish.ServerInfo()
	local cfg = Vanish.Config.status
	if cfg.spoof then
		return {version = cfg.version, address = boundAddress(),
			publicAddress = cfg.publicAddress, steamID = cfg.steamID}
	end

	-- a LAN server runs without Steam authentication, and the engine then
	-- prints neither the public address nor a steamid line
	local lan = GetConVar("sv_lan"):GetBool()
	local id64 = vanishinfo and vanishinfo.GetSteamID64()

	return {
		version = readVersion(),
		address = boundAddress(),
		publicAddress = not lan and vanishinfo and vanishinfo.GetPublicIP
			and vanishinfo.GetPublicIP() or nil,
		steamID = not lan and (id64 and renderSteamID(id64) or "not logged in") or nil,
	}
end
