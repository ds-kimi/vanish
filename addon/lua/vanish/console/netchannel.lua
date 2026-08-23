-- Maps a net channel back to the player that owns it.

---@param netchan CNetChan
---@return Player|nil ply owner of the netchannel, nil when unresolved
function Vanish.PlayerFromNetChan(netchan)
	if not netchan then return nil end

	local address = netchan:GetAddress()
	if not address then return nil end

	local text = address:ToString()
	for _, ply in ipairs(Vanish.RealGetAll()) do
		-- IPAddress() carries the client port too, so both sides match exactly
		if ply:IPAddress() == text then return ply end
	end

	return nil
end
