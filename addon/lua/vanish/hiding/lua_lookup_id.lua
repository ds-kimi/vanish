-- The remaining player lookups. GetByUniqueID is deprecated but still present
-- in old addons, and GetByID takes the UserID the status listing prints.

if Vanish.RealGetByUniqueID then
	---@param id string|number legacy unique ID
	---@return Player|nil
	function player.GetByUniqueID(id)
		return Vanish.Mask(Vanish.RealGetByUniqueID(id))
	end
end

---@param id number UserID as shown by status
---@return Player|nil
function player.GetByID(id)
	return Vanish.Mask(Vanish.RealGetByID(id))
end
