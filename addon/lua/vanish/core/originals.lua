-- Captures every library function the addon overrides, before it overrides
-- them. Internal code calls these so masking never hides a player from the
-- addon itself, and a missing entry means the running build has no such
-- function: the matching override then never installs.

Vanish.RealGetAll = Vanish.RealGetAll or player.GetAll
Vanish.RealGetHumans = Vanish.RealGetHumans or player.GetHumans
Vanish.RealPlayerIterator = Vanish.RealPlayerIterator or player.Iterator
Vanish.RealGetBySteamID = Vanish.RealGetBySteamID or player.GetBySteamID
Vanish.RealGetBySteamID64 = Vanish.RealGetBySteamID64 or player.GetBySteamID64
Vanish.RealGetByAccountID = Vanish.RealGetByAccountID or player.GetByAccountID
Vanish.RealGetByUniqueID = Vanish.RealGetByUniqueID or player.GetByUniqueID
Vanish.RealGetByID = Vanish.RealGetByID or player.GetByID

Vanish.RealTeamGetPlayers = Vanish.RealTeamGetPlayers or team.GetPlayers
Vanish.RealTeamNumPlayers = Vanish.RealTeamNumPlayers or team.NumPlayers
Vanish.RealTeamTotalFrags = Vanish.RealTeamTotalFrags or team.TotalFrags
Vanish.RealTeamTotalDeaths = Vanish.RealTeamTotalDeaths or team.TotalDeaths

Vanish.RealEntsGetAll = Vanish.RealEntsGetAll or ents.GetAll
Vanish.RealEntsIterator = Vanish.RealEntsIterator or ents.Iterator
Vanish.RealFindByClass = Vanish.RealFindByClass or ents.FindByClass
Vanish.RealFindByModel = Vanish.RealFindByModel or ents.FindByModel
Vanish.RealFindByName = Vanish.RealFindByName or ents.FindByName
Vanish.RealFindInSphere = Vanish.RealFindInSphere or ents.FindInSphere
Vanish.RealFindInBox = Vanish.RealFindInBox or ents.FindInBox
Vanish.RealFindInCone = Vanish.RealFindInCone or ents.FindInCone
Vanish.RealFindAlongRay = Vanish.RealFindAlongRay or ents.FindAlongRay
