-- The filtered ping reply, built from the masked roster.

--- Sends a ping listing that only contains visible players.
---@param ply Player requester
function Vanish.SendPing(ply)
	-- same masked roster as the status reply, so both stay consistent
	Vanish.SendLine(ply, "Client ping times:\n")

	-- the row format is the engine's own, and like the engine the listing
	-- skips bots, which have no connection to time
	for _, target in ipairs(player.GetHumans()) do
		Vanish.SendLine(ply, string.format("%4.0f ms : %s\n", target:Ping(), target:Nick()))
	end
end

Vanish.Replies["ping"] = Vanish.SendPing
