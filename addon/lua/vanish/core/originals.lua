-- Captures every library function the addon overrides, before it overrides
-- them. Internal code calls these so masking never hides a player from the
-- addon itself.
--
-- Only engine functions are listed. team.*, player.GetBySteamID and the other
-- Lua lookups are built on player.Iterator, whose cache is filled from the
-- masked player.GetAll, so they need no override of their own.

Vanish.RealGetAll = Vanish.RealGetAll or player.GetAll
Vanish.RealGetHumans = Vanish.RealGetHumans or player.GetHumans
Vanish.RealGetCount = Vanish.RealGetCount or player.GetCount
Vanish.RealGetByID = Vanish.RealGetByID or player.GetByID
Vanish.RealPlayer = Vanish.RealPlayer or Player

Vanish.RealEntsGetAll = Vanish.RealEntsGetAll or ents.GetAll
Vanish.RealFindByClass = Vanish.RealFindByClass or ents.FindByClass
Vanish.RealFindByModel = Vanish.RealFindByModel or ents.FindByModel
Vanish.RealFindByName = Vanish.RealFindByName or ents.FindByName
Vanish.RealFindInSphere = Vanish.RealFindInSphere or ents.FindInSphere
Vanish.RealFindInBox = Vanish.RealFindInBox or ents.FindInBox
Vanish.RealFindInCone = Vanish.RealFindInCone or ents.FindInCone
Vanish.RealFindAlongRay = Vanish.RealFindAlongRay or ents.FindAlongRay
