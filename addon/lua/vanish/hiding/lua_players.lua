-- Hides vanished players from the server side player library.

---@param list Entity[]
---@return Entity[] filtered list without any vanished player
function Vanish.Filter(list)
	if not Vanish.IsActive() then return list end

	local out = {}
	for i = 1, #list do
		local ent = list[i]
		if not (IsValid(ent) and ent:IsPlayer() and Vanish.IsVanished(ent)) then
			out[#out + 1] = ent
		end
	end

	return out
end

---@return Player[]
function player.GetAll()
	return Vanish.Filter(Vanish.RealGetAll())
end

---@return Player[]
function player.GetHumans()
	return Vanish.Filter(Vanish.RealGetHumans())
end

---@param ent Entity|nil result of a single entity lookup
---@return Entity|nil ent the entity itself, nil when it is a vanished player
function Vanish.Mask(ent)
	if IsValid(ent) and ent:IsPlayer() and Vanish.IsVanished(ent) then return nil end

	return ent
end
