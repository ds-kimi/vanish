-- Drives server side hook chains for a player that never actually left or
-- arrived. ULX, FAdmin and friends print from those hooks, so the fake is only
-- complete once they have run.

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

---@param event string hook name to collect handlers for
---@return table<any, function> snapshot of the handlers that take part
local function snapshot(event)
	local out = {}

	-- a handler may add or remove hooks while it runs, which would invalidate
	-- a live pairs() over the hook table
	for identifier, fn in pairs(hook.GetTable()[event] or {}) do
		-- the addon's own handlers are never part of the fake: they would
		-- undo the state the fake is announcing
		local own = isstring(identifier) and string.sub(identifier, 1, 7) == "vanish_"

		if not own and shouldRun(identifier) then out[identifier] = fn end
	end

	return out
end

--- Runs the selected handlers of one hook for a player whose real connection
--- state never changed.
---@param event string hook name
---@param ... any the arguments the engine passes to this hook
function Vanish.RunFakeHooks(event, ...)
	if Vanish.Config.disconnect.hooks == "none" then return end

	-- guards the addon's own cleanup and anything else that has to tell a
	-- fake from the real thing
	Vanish.Faking = true

	for identifier, fn in pairs(snapshot(event)) do
		-- object hooks are stored keyed by the object and called as methods
		local ok, err
		if istable(identifier) then
			ok, err = pcall(fn, identifier, ...)
		else
			ok, err = pcall(fn, ...)
		end

		if not ok and Vanish.Config.debug then
			print(string.format("[vanish] %s hook %s failed: %s",
				event, tostring(identifier), tostring(err)))
		end
	end

	Vanish.Faking = false
end
