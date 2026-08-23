-- Hides vanished players from the entity library.

---@param class string
---@return Entity[]
function ents.FindByClass(class)
	local found = Vanish.RealFindByClass(class)

	-- only the player class can contain a vanished entity, so everything else
	-- skips the copy entirely
	if class ~= "player" and class ~= "*" then return found end

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
