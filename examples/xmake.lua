-- Example project linking quickjs_wrapper library
-- Usage:
--   1. Build quickjs_wrapper first (run xmake in root directory)
--   2. Run xmake in this directory
--   3. Run: xmake run example_app

set_project("example_project")

add_rules("mode.debug", "mode.release")
set_languages("c++17")

-- Add quickjs dependency
add_requires("quickjs-ng", {alias = "quickjs", configs = {libc = true}})

-- Library path configuration
local LIB_ROOT = ".."

target("example_app")
    set_kind("binary")
    add_files("main.cpp")

    -- Include path
    add_includedirs(path.join(LIB_ROOT, "include"))

    -- Library path and linking
    add_linkdirs(path.join(LIB_ROOT, "lib", "$(arch)-$(mode)"))
    add_links("quickjs_wrapper")

    -- Dependencies
    add_packages("quickjs")
target_end()
