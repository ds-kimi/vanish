-- Hides vanished players from the server side player library.

--- Removes vanished players in place. Every caller passes the fresh table an
--- engine function just built, so nobody else holds a reference to it.
---@param list Entity[]
---@return Entity[] list the same table, without any vanished player
function Vanish.Filter(list)
	if not Vanish.IsActive() then return list end

	-- the entity keyed set holds players only, so a plain lookup answers
	-- for every kind of entity without an IsValid or IsPlayer call
	local vanished = Vanish.VanishedEnts
	local count = #list
	local kept = 0
	for i = 1, count do
		local ent = list[i]
		if not vanished[ent] then
			kept = kept + 1
			list[kept] = ent
		end
	end

	for i = kept + 1, count do list[i] = nil end

	return list
end

---@return Player[]
function player.GetAll()
	return Vanish.Filter(Vanish.RealGetAll())
end

---@return Player[]
function player.GetHumans()
	return Vanish.Filter(Vanish.RealGetHumans())
end

---@param ent Entity result of a single entity lookup
---@return Entity ent the entity itself, NULL when it is a vanished player
function Vanish.Mask(ent)
	-- the engine lookups return NULL rather than nil when nothing matches,
	-- and a hidden player has to look exactly like a missing one
	if Vanish.IsVanished(ent) then return NULL end

	return ent
end
