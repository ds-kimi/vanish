-- Hides vanished players from the team library. The counters matter as much as
-- the roster: a team whose player list is one shorter but whose count is not
-- tells any addon comparing them that somebody is hidden.

---@param id number team index
---@return Player[]
function team.GetPlayers(id)
	return Vanish.Filter(Vanish.RealTeamGetPlayers(id))
end

---@param id number team index
---@return number count of visible players on the team
function team.NumPlayers(id)
	return #team.GetPlayers(id)
end

---@param id number team index
---@return number frags summed over the visible players
function team.TotalFrags(id)
	local total = 0
	for _, ply in ipairs(team.GetPlayers(id)) do
		total = total + ply:Frags()
	end

	return total
end

---@param id number team index
---@return number deaths summed over the visible players
function team.TotalDeaths(id)
	local total = 0
	for _, ply in ipairs(team.GetPlayers(id)) do
		total = total + ply:Deaths()
	end

	return total
end
