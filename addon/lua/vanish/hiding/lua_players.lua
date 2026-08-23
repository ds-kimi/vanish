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

---@return number count of players visible to third-party code
function player.GetCount()
	return #player.GetAll()
end
