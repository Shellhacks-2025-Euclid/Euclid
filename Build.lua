workspace "Euclid"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }
	startproject "Euclid-App"

filter "system:windows"
	buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }
      
filter "system:macosx"
	architecture "ARM64"
	system "macosx"
	
filter {}

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Euclid-Lib"
	include "Euclid-Lib/Build-Euclid-Lib.lua"
	
group "Euclid-App"
	include "Euclid-App/Build-Euclid-App.lua"
	
group "Euclid-Lib-Debug"
	include "Euclid-Lib-Debug/Build-Euclid-Lib-Debug.lua"