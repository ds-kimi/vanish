-- Hides vanished players from the spatial entity searches. These are how an
-- addon finds who is standing somewhere without ever asking for the roster,
-- so they leak a vanished player's position as well as their existence.

---@param origin Vector
---@param radius number
---@return Entity[]
function ents.FindInSphere(origin, radius)
	return Vanish.Filter(Vanish.RealFindInSphere(origin, radius))
end

---@param mins Vector
---@param maxs Vector
---@return Entity[]
function ents.FindInBox(mins, maxs)
	return Vanish.Filter(Vanish.RealFindInBox(mins, maxs))
end

---@param origin Vector
---@param direction Vector
---@param radius number
---@param angleCos number
---@return Entity[]
function ents.FindInCone(origin, direction, radius, angleCos)
	return Vanish.Filter(Vanish.RealFindInCone(origin, direction, radius, angleCos))
end

if Vanish.RealFindAlongRay then
	---@param start Vector
	---@param stop Vector
	---@param mins Vector|nil
	---@param maxs Vector|nil
	---@return Entity[]
	function ents.FindAlongRay(start, stop, mins, maxs)
		return Vanish.Filter(Vanish.RealFindAlongRay(start, stop, mins, maxs))
	end
end
