-- premake5.lua
workspace "Lumiere"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Lumiere"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

-- group "Core"
-- 	include "Core/Build-Core.lua"
-- group ""

include "Lumiere/Build-Lumiere.lua"