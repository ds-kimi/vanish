-- Hides vanished players from the server side entity and team libraries.

---@param id number team index
---@return Player[]
function team.GetPlayers(id)
	return Vanish.Filter(Vanish.RealTeamGetPlayers(id))
end

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
