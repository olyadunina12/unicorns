workspace "unicorns"
	configurations { "Debug", "Release" }
	location "."
	architecture "x64"
	cppdialect "C++17"
	defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN" }

project "unicorns-client"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"

	files {
		"Client/**.*",
		"dependencies/imgui/*.cpp",
		"dependencies/imgui/*.h",
		"dependencies/imgui-sfml/*.cpp",
		"dependencies/imgui-sfml/*.h",
	}

	filter "configurations:*"
		defines { "SFML_STATIC" }
		includedirs {
			"dependencies/SFML/include",
			"dependencies/imgui",
			"dependencies/imgui-sfml",
		}
		libdirs { "dependencies/SFML/lib" }
		links
		{
			"unicorns-connect",
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

	files { "Server/**.*" }
	links {
		"unicorns-connect",
		"Ws2_32",
		"winmm",
	}

	filter "configurations:*"
		defines { "SFML_STATIC" }
		includedirs { "dependencies/SFML/include" }
		libdirs { "dependencies/SFML/lib" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		links
		{
			"sfml-system-s-d",
			"sfml-network-s-d"
		}

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		links
		{	
			"sfml-system-s",
			"sfml-network-s"
		}

project "unicorns-connect"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	includedirs { "dependencies/SFML/include" }
	libdirs { "dependencies/SFML/lib" }

	files { "Connect/**.*" }

	filter "configurations:*"
		defines { "SFML_STATIC" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		links
		{
			"sfml-network-s-d"
		}

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		links
		{	
			"sfml-network-s"
		}
