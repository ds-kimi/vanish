-- Keeps a vanished player out of chat. One message would undo the whole
-- illusion, and worse: the client was told this player disconnected, so an
-- addon printing chat from a player it believes gone can error client side.

---@param ply Player speaker
---@param text string message as typed
local function echoToStaff(ply, text)
	local line = string.format("(vanished) %s: %s\n", ply:Nick(), text)

	for _, viewer in ipairs(Vanish.RealGetAll()) do
		if viewer == ply or Vanish.IsAllowed(viewer) then
			viewer:PrintMessage(HUD_PRINTTALK, line)
		end
	end
end

hook.Add("PlayerSay", "vanish_chat", function(ply, text)
	if not Vanish.Config.silence.chat then return end
	if not Vanish.IsVanished(ply) then return end

	-- the message is echoed to staff and to the speaker before it is swallowed,
	-- so vanished admins can still talk without anybody else seeing it
	echoToStaff(ply, text)

	return ""
end)
