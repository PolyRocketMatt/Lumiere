project "LumiereCore"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
        "src",

      -- Additional includes
        "../Vendor/imgui",
        "../Vendor/glfw/include",
        "../Vendor/stb_image",
        
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glm}",
    }

    links 
    {
        "ImGui",
        "GLFW",

        "%{Library.Vulkan}"
    }

    -- Create virtual folders (filters)
   vpaths {
    ["Headers/*"] = { "**.h" },
    ["Sources/*"] = { "**.cpp"}
}

   targetdir ("../Binaries/" .. outputdir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

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