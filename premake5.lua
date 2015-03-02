if os.isfile("premake5.local.lua") then
	dofile "premake5.local.lua"
end

include "script"

PROJECT_NAME = path.getname(os.getcwd())

minko.project.solution(PROJECT_NAME)

	minko.project.application(PROJECT_NAME)

		files { "src/**.cpp", "src/**.hpp", "asset/**", "include/**.hpp" }
		includedirs { "src", "include" }

		-- plugin
		minko.plugin.enable("sdl")
		--minko.plugin.enable("bullet")
		minko.plugin.enable("serializer")
		minko.plugin.enable("particles")
		--minko.plugin.enable("fx")
		minko.plugin.enable("png")
		minko.plugin.enable("jpeg")
		minko.plugin.enable("oculus")
