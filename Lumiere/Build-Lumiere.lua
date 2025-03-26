GLFW_PATH = "C:/Dev/C++/libs/glfw-3.4.bin.WIN64/"
GLM_PATH = "C:/Dev/C++/libs/glm/"
IMGUI_PATH = "C:/Dev/C++/libs/imgui/"
VULKAN_PATH = "C:/VulkanSDK/1.4.304.1/"
SPDLOG_PATH = "C:/Dev/C++/libs/spdlog/"

project "Lumiere"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    files { "Source/**.h", "Source/**.cpp", "Source/**.hpp" }

    includedirs
    {
            "Source",

            -- Include Core
            -- "../Core/Source"

            -- Include GLFW and GLM
            GLFW_PATH .. "include",
            GLM_PATH,

            -- Include ImGui
            IMGUI_PATH,

            -- Include Vulkan
            VULKAN_PATH .. "Include",    

            -- Include spdlog
            SPDLOG_PATH .. "include"
    }

    links
    {
        -- "Core"

        -- Link GLFW
        GLFW_PATH .. "lib-vc2022/glfw3.lib",
        
        -- Link Vulkan
        VULKAN_PATH .. "Lib/vulkan-1.lib"
    }

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }

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