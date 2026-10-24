-- Blanks the userinfo string table entry of a vanished player. That entry is
-- the last place a client still holds their name and SteamID after the entity
-- is gone: no client Lua API reads it, but a client side binary module does,
-- and it is the one surface the rest of the addon cannot reach.

--- Matches the blanked entries to the current vanish state.
function Vanish.RefreshUserInfo()
	if not vanishuserinfo or not Vanish.Config.hideUserinfo then return end

	for _, ply in ipairs(Vanish.RealGetAll()) do
		vanishuserinfo.SetHidden(ply:EntIndex(), Vanish.IsVanished(ply))
	end
end

-- entries are keyed by client slot and slots are reused, so a leaving player
-- must have theirs restored before the next client takes it
hook.Add("PlayerDisconnected", "vanish_userinfo_cleanup", function(ply)
	if not vanishuserinfo or Vanish.Faking then return end

	if IsValid(ply) then vanishuserinfo.SetHidden(ply:EntIndex(), false) end
end)

-- the engine rewrites the entry whenever the client changes a userinfo
-- convar, its name included, which puts the real name back on every client;
-- the module saves that newer copy and blanks it again on the next pass
timer.Create("vanish_userinfo_refresh", Vanish.Config.refreshInterval, 0, function()
	if Vanish.IsActive() then Vanish.RefreshUserInfo() end
end)

if not vanishuserinfo and Vanish.Config.hideUserinfo then
	ErrorNoHalt("[vanish] userinfo module unavailable, string table entries stay\n")
end
