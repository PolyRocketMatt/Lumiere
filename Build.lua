-- premake5.lua
workspace "Lumiere"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "LumiereApp"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

include "Vendor.lua"

group "Core"
	include "LumiereCore/LumiereCore.lua"
group ""

include "LumiereApp/LumiereApp.lua"