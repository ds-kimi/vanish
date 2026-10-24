-- Hides vanished players from the entity library.

---@param class string
---@return Entity[]
function ents.FindByClass(class)
	local found = Vanish.RealFindByClass(class)

	-- only a pattern that can match the player class can return a vanished
	-- entity, so anything else skips the pass. The engine matches class names
	-- case insensitively and with * wildcards, so "Player" and "pl*" can
	if not isstring(class) then return found end
	if not string.find(class, "*", 1, true) and string.lower(class) ~= "player" then return found end

	return Vanish.Filter(found)
end

---@return Entity[]
function ents.GetAll()
	return Vanish.Filter(Vanish.RealEntsGetAll())
end

---@param model string
---@return Entity[]
function ents.FindByModel(model)
	return Vanish.Filter(Vanish.RealFindByModel(model))
end

---@param name string targetname
---@return Entity[]
function ents.FindByName(name)
	return Vanish.Filter(Vanish.RealFindByName(name))
end
