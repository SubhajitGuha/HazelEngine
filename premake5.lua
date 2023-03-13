workspace "Hazel_Engine"

architecture "x86_64"

startproject "Sandbox"

configurations
{
	"Debug",
	"Release",
	"Dist"
}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Hazel/vendor/GLFW/include"
IncludeDir["Glad"] = "Hazel/vendor/Glad/include"
IncludeDir["imgui"] = "Hazel/vendor/imgui"
IncludeDir["glm"] = "Hazel/vendor/glm"
IncludeDir["stb_image"] = "Hazel/vendor/stb_image"
IncludeDir["entt"] = "Hazel/vendor/entt"
IncludeDir["curl"] = "Hazel/vendor/Curl/include"
IncludeDir["json"]="Hazel/vendor/jsoncpp"
IncludeDir["assimp"]="Hazel/vendor/assimp/include"

include "Hazel/vendor/GLFW"
include "Hazel/vendor/Glad"
include "Hazel/vendor/imgui"

project "Hazel"

	location "Hazel"
	kind "StaticLib"
	language "c++"
	staticruntime "on"
	cppdialect "c++17"

	targetdir ("bin/"..outputdir.."/%{prj.name}")
	objdir ("bin-int/"..outputdir.."/%{prj.name}")


	pchheader "hzpch.h"
	pchsource "%{prj.name}/src/hzpch.cpp"


	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{IncludeDir.stb_image}/**.h",
		"%{IncludeDir.stb_image}/**.cpp",
		"%{IncludeDir.curl}/**.h",
		"%{IncludeDir.curl}/**.c",
		"%{IncludeDir.curl}/**.cpp",
		"%{IncludeDir.json}/**.h",
		"%{IncludeDir.json}/**.cpp",
		"%{IncludeDir.json}/**.c",
		"%{IncludeDir.entt}/**.hpp",
		"%{IncludeDir.assimp}/**.h",
		"%{IncludeDir.assimp}/**.cpp",
		"%{IncludeDir.assimp}/**.hpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.curl}",
		"%{IncludeDir.json}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}"
	}

	links{
		"GLFW",
		"Glad",
		"imgui",
		"opengl32.lib",
		"Normaliz.lib",
		"Ws2_32.lib",
		"Wldap32.lib",
		"Crypt32.lib",
		"advapi32.lib",
		"Hazel/vendor/Curl/lib/libcurl_a_debug.lib",
		"Hazel/vendor/assimp/lib/Release/assimp-vc142-mt.lib"
	}

	filter "system:windows"
		
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}


	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "On"
	

project "Sandbox"

	location "Sandbox"
	kind "ConsoleApp"
	language "c++"
	staticruntime "on"
	cppdialect "c++17"

	targetdir ("bin/"..outputdir.."/%{prj.name}")
	objdir ("bin-int/"..outputdir.."/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/Assets/**.png"
	}
	includedirs
	{
		"Hazel/vendor/spdlog/include",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.curl}",
		"%{IncludeDir.json}",
		"Hazel/src"
	}
	links "Hazel"

	filter "system:windows"
		
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS"
		}
		
	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "On"


project "Hazel_Editor"

	location "Hazel_Editor"
	kind "ConsoleApp"
	language "c++"
	staticruntime "on"
	cppdialect "c++17"

	targetdir ("bin/"..outputdir.."/%{prj.name}")
	objdir ("bin-int/"..outputdir.."/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/Assets/**.png"
	}
	includedirs
	{
		"Hazel/vendor/spdlog/include",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.curl}",
		"%{IncludeDir.json}",
		"Hazel/src"
	}
	links "Hazel"

	filter "system:windows"
		
		systemversion "latest"

		defines
		{
			"HZ_PLATFORM_WINDOWS"
		}
		
	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "On"
