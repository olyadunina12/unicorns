workspace "unicorns"
	configurations { "Debug", "Release" }
	location "."
	architecture "x64"

project "unicorns-client"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"

	files { "Client/**.*" }

	filter "configurations:*"
		defines { "SFML_STATIC" }
		includedirs { "dependencies/SFML/include" }
		libdirs { "dependencies/SFML/lib" }
		links
		{
			"opengl32",
			"freetype",
			"winmm",
			"gdi32",
			"flac",
			"vorbisenc",
			"vorbisfile",
			"vorbis",
			"ogg",
			"ws2_32"
		}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		links
		{	
			"sfml-graphics-s-d",
			"sfml-window-s-d",
			"sfml-system-s-d",
			"sfml-audio-s-d",
			"sfml-network-s-d"
		}

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}

project "unicorns-server"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"

	files { "Client/**.*" }

	filter "configurations:*"
		defines { "SFML_STATIC" }
		includedirs { "dependencies/SFML/include" }
		libdirs { "dependencies/SFML/lib" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		links
		{	
			-- "sfml-system-s-d",
			"sfml-network-s-d"
		}

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		links
		{	
			-- "sfml-system-s",
			"sfml-network-s"
		}


project "unicorns-connect"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"

	files { "Connect/**.*" }

	filter "configurations:*"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
