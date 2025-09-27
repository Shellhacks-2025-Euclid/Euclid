project "Euclid-Lib"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	targetdir "Binaries/%{cfg.buildcfg}"
	staticruntime "off"

files { "Include/**.hpp", "Source/**.cpp", "Include/**.h", "Source/**.c", "Wrapper/**.h", "Wrapper/**.hpp", "Wrapper/**.cpp" }

includedirs { "Source", "Include", "Wrapper/Include" }
   
externalincludedirs { "../Dependencies/include" }

targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")



filter "system:windows"
       systemversion "latest"
       defines {}
       libdirs { "../Dependencies/lib" }
       links   { "glfw3" }
       
filter "system:macosx"
       defines { "MACOS" }
       architecture "ARM64"
       system "macosx"
       libdirs { "../Dependencies/lib-arm64" }
	links   {  
        "glfw3",
        "OpenGL.framework",
        "Cocoa.framework",
        "IOKit.framework",
        "CoreVideo.framework",
        "CoreFoundation.framework"
	}
filter {} 

filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"