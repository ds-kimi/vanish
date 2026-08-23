-- Drops the console commands that would list a vanished player.

include("sourcenet/incoming.lua")

---@param cmd string raw command line sent by the client
---@return boolean blocked whether this command enumerates connected players
local function isListingCommand(cmd)
	local name = string.match(string.lower(cmd), "^%s*([%w_]+)")
	if not name then return false end

	return Vanish.Config.blockedCommands[name] == true
end

-- client console commands the engine forwards to the server arrive here before
-- the server ever executes them, so a listing command can be dropped outright
FilterIncomingMessage(net_StringCmd, function(netchan, read, write)
	local cmd = read:ReadString()

	if isListingCommand(cmd) then
		local sender = Vanish.PlayerFromNetChan(netchan)

		if Vanish.Config.debug then
			print(string.format("[vanish] %q from %s active=%s allowed=%s",
				cmd, IsValid(sender) and sender:Nick() or "<unresolved>",
				tostring(Vanish.IsActive()), tostring(Vanish.IsAllowed(sender))))
		end
	end

	if Vanish.IsActive() and Vanish.Config.filterCommands and isListingCommand(cmd) then
		local sender = Vanish.PlayerFromNetChan(netchan)

		-- staff keep the real engine output; everyone else has the command
		-- dropped here and, for status, answered with a filtered listing so
		-- that silence never hints at something being hidden
		if not Vanish.IsAllowed(sender) then
			-- answering is deferred out of the packet handler: writing to a
			-- netchannel while its incoming datagram is still being filtered
			-- is what corrupted the reliable stream before
			local name = string.match(string.lower(cmd), "^%s*([%w_]+)") or ""
			if IsValid(sender) and Vanish.Replies[name] then
				local reply = Vanish.Replies[name]
				timer.Simple(0, function()
					if IsValid(sender) then reply(sender) end
				end)
			end

			-- the message is dropped entirely: rewriting it to a harmless
			-- command made the client print "Unknown command". The rewrite is
			-- only adopted once the whole datagram parsed, so dropping cannot
			-- desync the rest of it (see MarkIncomingModified)
			MarkIncomingModified()
			return
		end
	end

	write:WriteUInt(net_StringCmd, NET_MESSAGE_BITS)
	write:WriteString(cmd)
end)
