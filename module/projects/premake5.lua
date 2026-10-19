PROJECT_GENERATOR_VERSION = 2

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
		files({"../source/**.cpp", "../source/**.hpp"})
		includedirs({"../source"})

		-- code patching and ELF parsing, for the hooks the Linux engine
		-- leaves no function to detour for
		filter("system:not linux")
			removefiles({"../source/platform/**"})

		filter("system:linux")
			links({"dl"})

		filter({})
