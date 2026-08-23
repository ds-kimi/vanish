-- Hides vanished players from the single player lookups. Each one is a way to
-- reach a player without ever listing the roster, so filtering only the list
-- functions would leave every one of them wide open.

---@return number count of players visible to third-party code
function player.GetCount()
	return #player.GetAll()
end

---@param id string SteamID in the STEAM_0:0:0 form
---@return Player|nil
function player.GetBySteamID(id)
	return Vanish.Mask(Vanish.RealGetBySteamID(id))
end

---@param id string SteamID64 as a string
---@return Player|nil
function player.GetBySteamID64(id)
	return Vanish.Mask(Vanish.RealGetBySteamID64(id))
end

---@param id number Steam account ID
---@return Player|nil
function player.GetByAccountID(id)
	return Vanish.Mask(Vanish.RealGetByAccountID(id))
end
