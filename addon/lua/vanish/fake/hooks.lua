-- Drives the server side PlayerDisconnected handlers so admin mods log the
-- departure. ULX, SAM and friends print from that hook, so the fake is only
-- complete once they have run for a player that never actually left.

---@param identifier any hook identifier, a string or an object for method hooks
---@return boolean run whether this handler is part of the fake
local function shouldRun(identifier)
	local cfg = Vanish.Config.disconnect

	if cfg.hooks == "all" then return true end
	if cfg.hooks ~= "match" then return false end

	local name = string.lower(tostring(identifier))
	for _, pattern in ipairs(cfg.hookPatterns) do
		if string.find(name, pattern, 1, true) then return true end
	end

	return false
end

---@return table<any, function> snapshot of the current PlayerDisconnected hooks
local function snapshot()
	local out = {}

	-- a handler may add or remove hooks while it runs, which would invalidate
	-- a live pairs() over the hook table
	for identifier, fn in pairs(hook.GetTable()["PlayerDisconnected"] or {}) do
		if identifier ~= "vanish_cleanup" and shouldRun(identifier) then
			out[identifier] = fn
		end
	end

	return out
end

--- Runs the selected PlayerDisconnected handlers for a player that stays on
--- the server.
---@param ply Player
function Vanish.RunDisconnectHooks(ply)
	if Vanish.Config.disconnect.hooks == "none" then return end

	-- guards the addon's own cleanup and anything else that has to tell a
	-- fake disconnect from the real one
	Vanish.Faking = true

	for identifier, fn in pairs(snapshot()) do
		-- object hooks are stored keyed by the object and called as methods
		local ok, err
		if istable(identifier) then
			ok, err = pcall(fn, identifier, ply)
		else
			ok, err = pcall(fn, ply)
		end

		if not ok and Vanish.Config.debug then
			print(string.format("[vanish] disconnect hook %s failed: %s",
				tostring(identifier), tostring(err)))
		end
	end

	Vanish.Faking = false
end
