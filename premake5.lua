-- =============================================================================
--  Hazel Engine  --  premake5.lua
--  重构要点：
--    1. 公共配置提取为函数，消除跨 project 的重复块
--    2. 第三方库目录 / 库文件集中在顶部统一管理
--    3. 每个 project 只声明自身独有的内容
-- =============================================================================

workspace "Hazel"
    architecture "x64"
    startproject "Hazelnut"

    configurations { "Debug", "Release", "Dist" }

-- -----------------------------------------------------------------------------
--  全局路径
-- -----------------------------------------------------------------------------

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

local VULKAN_SDK = os.getenv("VULKAN_SDK")

-- 第三方头文件目录
IncludeDir = {
    GLFW          = "Hazel/vendor/GLFW/include",
    GLAD          = "Hazel/vendor/GLAD/include",
    ImGui         = "Hazel/vendor/imgui",
    glm           = "Hazel/vendor/glm",
    stb_image     = "Hazel/vendor/stb_image",
    entt          = "Hazel/vendor/entt/include",
    tinyobjloader = "Hazel/vendor/tinyobjloader/include",
    yaml_cpp      = "Hazel/vendor/yaml-cpp/include",
    ImGuizmo      = "Hazel/vendor/ImGuizmo",
    Box2D         = "Hazel/vendor/Box2D/include",
    mono          = "Hazel/vendor/mono/include",
    miniaudio     = "Hazel/vendor/miniaudio",
    VulkanSDK     = VULKAN_SDK .. "/Include",
}

-- 第三方静态库
Library = {
    -- Vulkan / shader compiler
    ShaderC_Debug          = VULKAN_SDK .. "/Lib/shaderc_sharedd.lib",
    SPIRV_Cross_Debug      = VULKAN_SDK .. "/Lib/spirv-cross-cored.lib",
    SPIRV_Cross_GLSL_Debug = VULKAN_SDK .. "/Lib/spirv-cross-glsld.lib",

    ShaderC_Release          = VULKAN_SDK .. "/Lib/shaderc_shared.lib",
    SPIRV_Cross_Release      = VULKAN_SDK .. "/Lib/spirv-cross-core.lib",
    SPIRV_Cross_GLSL_Release = VULKAN_SDK .. "/Lib/spirv-cross-glsl.lib",

    -- Mono
    mono_Debug   = "%{wks.location}/Hazel/vendor/mono/lib/Debug/libmono-static-sgen.lib",
    mono_Release = "%{wks.location}/Hazel/vendor/mono/lib/Release/libmono-static-sgen.lib",
}

-- -----------------------------------------------------------------------------
--  公共配置函数
-- -----------------------------------------------------------------------------

--- 所有项目共用的基础设置（语言、标准、输出目录、警告选项）
function common_settings()
    language   "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("bin/"     .. outputdir .. "/%{prj.name}")
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    defines { "_CRT_SECURE_NO_WARNINGS" }

    filter "system:windows"
        systemversion "latest"
        buildoptions  { "/utf-8", "/wd4828" }
        defines       { "HZ_PLATFORM_WINDOWS" }
    filter {}
end

--- Debug / Release / Dist 三档配置（symbols、optimize、预处理宏）
--- 接受两张可选的"按配置链接库"表，分别对应 Debug 和 Release/Dist。
function config_filters(debug_libs, release_libs)
    debug_libs   = debug_libs   or {}
    release_libs = release_libs or {}

    filter "configurations:Debug"
        defines "HZ_DEBUG"
        runtime "Debug"
        symbols "on"
        links   (debug_libs)

    filter "configurations:Release"
        defines  "HZ_RELEASE"
        runtime  "Release"
        optimize "on"
        links    (release_libs)

    filter "configurations:Dist"
        defines  "HZ_DIST"
        runtime  "Release"
        optimize "on"
        links    (release_libs)

    filter {}   -- 重置 filter，避免污染后续声明
end

-- -----------------------------------------------------------------------------
--  依赖子项目
-- -----------------------------------------------------------------------------

group "Dependencies"
    include "Hazel/vendor/GLFW"
    include "Hazel/vendor/GLAD"
    include "Hazel/vendor/imgui"
    include "Hazel/vendor/yaml-cpp"
    include "Hazel/vendor/Box2D"
group ""

-- -----------------------------------------------------------------------------
--  project: Hazel  （引擎静态库）
-- -----------------------------------------------------------------------------

project "Hazel"
    location "Hazel"
    kind     "StaticLib"
    common_settings()

    pchheader "hzpch.h"
    pchsource "Hazel/src/hzpch.cpp"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/stb_image/**.h",
        "%{prj.name}/vendor/stb_image/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",
        "%{prj.name}/vendor/glm/glm/**.inl",
        "%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
        "%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp",
    }

    includedirs {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.tinyobjloader}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.mono}",
        "%{IncludeDir.miniaudio}",
    }

    links {
        "GLFW", "GLAD", "ImGui",
        "opengl32.lib",
        "yaml-cpp", "Box2D",
    }

    -- ImGuizmo 是第三方 cpp，不参与 PCH
    filter "files:Hazel/vendor/ImGuizmo/**.cpp"
        enablepch "Off"
    filter {}

    -- Windows 额外系统库
    filter "system:windows"
        defines { "HZ_BUILD_DLL", "GLFW_INCLUDE_NONE" }
        links   { "Ws2_32", "Winmm", "Version", "Bcrypt" }
    filter {}

    config_filters(
        -- Debug
        {
            Library.ShaderC_Debug,
            Library.SPIRV_Cross_Debug,
            Library.SPIRV_Cross_GLSL_Debug,
            Library.mono_Debug,
        },
        -- Release / Dist
        {
            Library.ShaderC_Release,
            Library.SPIRV_Cross_Release,
            Library.SPIRV_Cross_GLSL_Release,
            Library.mono_Release,
        }
    )

-- -----------------------------------------------------------------------------
--  project: Sandbox  （示例/测试程序）
-- -----------------------------------------------------------------------------

project "Sandbox"
    location "Sandbox"
    kind     "ConsoleApp"
    common_settings()

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs {
        "Hazel/vendor/spdlog/include",
        "Hazel/src",
        "Hazel/vendor",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.tinyobjloader}",
    }

    links { "Hazel" }

    config_filters()    -- 无额外按配置链接库

-- -----------------------------------------------------------------------------
--  project: Hazelnut  （编辑器可执行程序）
-- -----------------------------------------------------------------------------

project "Hazelnut"
    location "Hazelnut"
    kind     "ConsoleApp"
    common_settings()

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs {
        "Hazel/vendor/spdlog/include",
        "Hazel/src",
        "Hazel/vendor",
        "Hazelnut/src",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.tinyobjloader}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.miniaudio}",
    }

    links { "Hazel" }

    config_filters()    -- 无额外按配置链接库