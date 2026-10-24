-- Both modules are optional. Each feature checks for its own module and
-- degrades on its own, so a missing one never stops the addon loading.
pcall(require, "vanish")
pcall(require, "sourcenet")

-- Load order matters twice: core/originals captures the untouched library
-- functions before hiding/lua_* replaces them, and console/status registers
-- the reply that console/filter dispatches.
local FILES = {
	"vanish/sv_config.lua",
	"vanish/core/originals.lua",

	"vanish/core/state.lua",
	"vanish/core/toggle.lua",

	"vanish/hiding/entities.lua",
	"vanish/hiding/lua_players.lua",
	"vanish/hiding/lua_lookup.lua",
	"vanish/hiding/lua_entities.lua",
	"vanish/hiding/lua_find.lua",
	"vanish/hiding/queries.lua",
	"vanish/hiding/userinfo.lua",

	"vanish/hiding/sound.lua",
	"vanish/hiding/voice.lua",
	"vanish/hiding/chat.lua",
	"vanish/hiding/combat.lua",

	"vanish/fake/hooks.lua",
	"vanish/fake/hook_runs.lua",
	"vanish/fake/disconnect.lua",
	"vanish/fake/protect.lua",

	"vanish/core/immediate.lua",
	"vanish/core/persist.lua",

	"vanish/console/netchannel.lua",
	"vanish/console/serverinfo.lua",
	"vanish/console/status.lua",
	"vanish/console/ping.lua",
	"vanish/console/users.lua",
	"vanish/console/stats.lua",
	"vanish/console/filter.lua",

	"vanish/core/command.lua",
}

for _, path in ipairs(FILES) do
	include(path)
end
