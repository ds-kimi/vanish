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

-- Seconds between two synthesised replies to the same player. Each reply is a
-- burst of reliable messages built over the whole roster, so without a limit
-- a client could send hundreds of these commands per packet and have the
-- server do that work for every one of them.
local REPLY_COOLDOWN = 1

---@type table<Player, number> next CurTime a player may be answered again
local nextReply = setmetatable({}, {__mode = "k"})

--- Answers a dropped command with its filtered listing, at most once per
--- cooldown per player. Over the limit the command is still dropped, silently.
---@param sender Player
---@param name string lowercased command name
local function answer(sender, name)
	local reply = Vanish.Replies[name]
	if not IsValid(sender) or not reply then return end

	local now = CurTime()
	if (nextReply[sender] or 0) > now then return end
	nextReply[sender] = now + REPLY_COOLDOWN

	-- answering is deferred out of the packet handler: writing to a
	-- netchannel while its incoming datagram is still being filtered is
	-- what corrupted the reliable stream before
	timer.Simple(0, function()
		if IsValid(sender) then reply(sender) end
	end)
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
		-- never hints at something being hidden. The message is dropped
		-- entirely: rewriting it made the client print "Unknown command", and
		-- the rewrite is only adopted once the whole datagram parsed
		if Vanish.Config.filterCommands and Vanish.IsActive() and not Vanish.IsAllowed(sender) then
			answer(sender, name)
			MarkIncomingModified()
			return
		end
	end

	write:WriteUInt(net_StringCmd, NET_MESSAGE_BITS)
	write:WriteString(cmd)
end)
