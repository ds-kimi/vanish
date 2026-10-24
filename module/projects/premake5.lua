-- version 3 adds the x86_64 platform next to x86, so one generated project
-- builds gmsv_vanish_win32/_win64 and gmsv_vanish_linux/_linux64
PROJECT_GENERATOR_VERSION = 3

newoption({
	trigger = "gmcommon",
	description = "Sets the path to the garrysmod_common directory",
	value = "path to garrysmod_common directory"
})

local gmcommon = assert(_OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON"),
	"provide a path to garrysmod_common with --gmcommon or GARRYSMOD_COMMON")
include(gmcommon)

CreateWorkspace({name = "vanish", abi_compatible = true})
	CreateProject({serverside = true})
		IncludeLuaShared()
		IncludeHelpersExtended()
		IncludeSDKCommon()
		IncludeSDKTier0()
		IncludeSDKTier1()
		IncludeSteamAPI()
		IncludeDetouring()
		IncludeScanning()
		-- platform specific files guard themselves with #if, so every
		-- configuration compiles the same file list
		files({"../source/**.cpp", "../source/**.hpp"})
		includedirs({"../source"})

		filter("system:linux")
			links({"dl"})
			-- only gmod13_open and gmod13_close have to be visible. Everything
			-- else, the bundled minhook and SDK code included, stays local so it
			-- can neither interpose on nor be interposed by another module
			buildoptions({"-fvisibility=hidden", "-fvisibility-inlines-hidden"})
			linkoptions({"-Wl,--exclude-libs,ALL", "-Wl,-z,relro,-z,now", "-Wl,-z,noexecstack"})

		filter({})
