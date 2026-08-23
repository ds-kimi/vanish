-- The filtered ping reply, built from the masked roster.

--- Sends a ping listing that only contains visible players.
---@param ply Player requester
function Vanish.SendPing(ply)
	-- same masked roster as the status reply, so both stay consistent
	Vanish.SendLine(ply, "Client ping times:\n")

	for _, target in ipairs(player.GetAll()) do
		Vanish.SendLine(ply, string.format("  %i ms : %s\n", target:Ping(), target:Nick()))
	end
end

Vanish.Replies["ping"] = Vanish.SendPing
