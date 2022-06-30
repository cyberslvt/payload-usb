workspace "pscript"
	architecture "x64"
	flags {"MultiProcessorCompile"}

	configurations 
	{
		"Debug", 
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}"

project "p-compiler"
	location "Compiler"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir.. "/%{prj.name}")
	debugdir "%{cfg.targetdir}"

	debugargs { "test.pscript" }
	
	postbuildcommands {
		"{MKDIR} %{cfg.targetdir}/resources",
		"{COPYDIR} resources %{cfg.targetdir}/resources",
		"{COPYFILE} test.pscript %{cfg.targetdir}/"
	}

	files 
	{
		"Compiler/src/**.h",
		"Compiler/src/**.cpp"
	}

	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"
		staticruntime "on"
		runtime "Debug"

	filter "configurations:Release"
		defines "RELEASE"
		optimize "On"
		staticruntime "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "DIST"
		optimize "On"
		staticruntime "on"
		runtime "Release"

project "p-emulator"
	location "Emulator"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir.. "/%{prj.name}")
	debugdir "%{cfg.targetdir}"
	debugargs { "test.bin" }

	files 
	{
		"Emulator/src/**.h",
		"Emulator/src/**.cpp"
	}

	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"
		staticruntime "on"
		runtime "Debug"

	filter "configurations:Release"
		defines "RELEASE"
		optimize "On"
		staticruntime "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "DIST"
		optimize "On"
		staticruntime "on"
		runtime "Release"