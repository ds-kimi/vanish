-- Drops the console commands that would list a vanished player.

-- sourcenet ships the Lua half of its netchannel filtering. Without it there
-- is no way to see a client command before the engine runs it, so the feature
-- reports itself missing and stops: the rest of the addon is unaffected.
if not pcall(include, "sourcenet/incoming.lua") or not FilterIncomingMessage then
	if Vanish.Config.filterCommands then
		ErrorNoHalt("[vanish] sourcenet unavailable, status and ping are unfiltered\n")
	end

	return
end

---@param cmd string raw command line sent by the client
---@return string|nil name the command the engine will run, lowercased
local function commandName(cmd)
	-- the engine's tokenizer strips a quote around the first word, so
	-- "status" in quotes runs status and has to match here too
	return string.match(string.lower(cmd), '^%s*"?([%w_]+)')
end

-- client console commands the engine forwards to the server arrive here before
-- the server ever executes them, so a listing command can be dropped outright
FilterIncomingMessage(net_StringCmd, function(netchan, read, write)
	local cmd = read:ReadString()
	local name = commandName(cmd)

	if name and Vanish.Config.blockedCommands[name] then
		local sender = Vanish.PlayerFromNetChan(netchan)

		if Vanish.Config.debug then
			print(string.format("[vanish] %q from %s active=%s allowed=%s",
				cmd, IsValid(sender) and sender:Nick() or "<unresolved>",
				tostring(Vanish.IsActive()), tostring(Vanish.IsAllowed(sender))))
		end

		-- staff keep the real engine output; everyone else has the command
		-- dropped here and answered with a filtered listing, so that silence
		-- never hints at something being hidden
		if Vanish.Config.filterCommands and Vanish.IsActive() and not Vanish.IsAllowed(sender) then
			local reply = Vanish.Replies[name]

			-- answering is deferred out of the packet handler: writing to a
			-- netchannel while its incoming datagram is still being filtered
			-- is what corrupted the reliable stream before
			if IsValid(sender) and reply then
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
