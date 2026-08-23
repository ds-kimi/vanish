-- Stops a vanished player's voice reaching other clients. The voice packet
-- carries the speaker's entity index, so a client that receives it pops the
-- speaking panel with the name from its own player list.

hook.Add("PlayerCanHearPlayersVoice", "vanish_voice", function(listener, talker)
	if not Vanish.Config.silence.voice then return end
	if not Vanish.IsVanished(talker) then return end

	-- staff keep hearing a vanished player, so two hidden admins can still
	-- talk to each other
	if Vanish.IsAllowed(listener) then return end

	return false
end)
