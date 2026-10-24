-- Hides vanished players from the engine's single player lookups. Each one is
-- a way to reach a player without ever listing the roster.

---@return number count of players visible to third-party code
function player.GetCount()
	-- the engine counts without building a table, which matters while
	-- nobody is vanished and this runs as often as addons call it
	if not Vanish.IsActive() then return Vanish.RealGetCount() end

	return #player.GetAll()
end

---@param index number entity index
---@return Player
function player.GetByID(index)
	return Vanish.Mask(Vanish.RealGetByID(index))
end

---@param userid number UserID as shown by status
---@return Player
function Player(userid)
	return Vanish.Mask(Vanish.RealPlayer(userid))
end
