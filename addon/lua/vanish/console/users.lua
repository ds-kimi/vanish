-- The filtered users reply, built from the masked roster.
--
-- Dropping the command without answering it is its own tell: a client that
-- types users and gets nothing back knows something ate the command. The
-- formats below are the engine's own, read out of engine.dll.

--- Sends a users listing that only contains visible players.
---@param ply Player requester
function Vanish.SendUsers(ply)
	local visible = player.GetAll()

	Vanish.SendLine(ply, "<slot:userid:\"name\">\n")

	for _, target in ipairs(visible) do
		-- the slot is the entity index without the world entity
		Vanish.SendLine(ply, string.format("%i:%i:\"%s\"\n",
			target:EntIndex() - 1, target:UserID(), target:Nick()))
	end

	Vanish.SendLine(ply, string.format("%i users\n", #visible))
end

Vanish.Replies["users"] = Vanish.SendUsers
