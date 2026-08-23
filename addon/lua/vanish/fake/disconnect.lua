-- Broadcasts the engine events that make a vanish read as a real disconnect.
-- Clearing the transmit bit already removes the entity from every client, but
-- only the game event makes a client run its own leave path: the chat notice,
-- the scoreboard entry and any addon listening for player_disconnect.

---@param ply Player
---@return table fields matching the engine player_disconnect descriptor
local function disconnectFields(ply)
	return {
		userid = ply:UserID(),
		reason = Vanish.Config.disconnect.reason,
		name = ply:Nick(),
		networkid = ply:SteamID(),
		bot = ply:IsBot() and 1 or 0,
	}
end

---@param ply Player
---@return table fields matching the engine player_connect descriptor
local function connectFields(ply)
	return {
		userid = ply:UserID(),
		-- the event carries the client slot, which is the entity index minus
		-- the world entity
		index = ply:EntIndex() - 1,
		name = ply:Nick(),
		networkid = ply:SteamID(),
		-- the engine sends "none" for a real client too, so no address of a
		-- player ever reaches the other clients through this
		address = "none",
		bot = ply:IsBot() and 1 or 0,
	}
end

--- Tells every client the player left, without the server dropping them.
---@param ply Player
function Vanish.FakeDisconnect(ply)
	if not Vanish.Config.fakeDisconnect or not vanishevent then return end
	if not IsValid(ply) or not ply:IsPlayer() then return end

	vanishevent.Fire("player_disconnect", disconnectFields(ply))
	Vanish.RunDisconnectHooks(ply)
end

--- Tells every client the player joined, so the reappearing entity has a
--- matching announcement instead of showing up unexplained.
---@param ply Player
function Vanish.FakeRejoin(ply)
	if not Vanish.Config.fakeDisconnect or not vanishevent then return end
	if not Vanish.Config.disconnect.announceRejoin then return end
	if not IsValid(ply) or not ply:IsPlayer() then return end

	vanishevent.Fire("player_connect", connectFields(ply))
end

if not vanishevent and Vanish.Config.fakeDisconnect then
	ErrorNoHalt("[vanish] event module unavailable, no fake disconnect is sent\n")
end
