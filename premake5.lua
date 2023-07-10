workspace "2v2pl"
  configurations { "Default" }
  startproject "2v2pl"
  location ("build/projects/" .. _ACTION)

  project "2v2pl"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    location ("build/projects/" .. _ACTION .. "/%{prj.name}")

    targetdir "build/bin/%{cfg.system}/%{prj.name}"
    objdir "build/obj/%{cfg.system}/%{prj.name}"

    files { "src/**.hpp", "src/**.cpp" }
