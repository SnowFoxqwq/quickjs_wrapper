set_project("quickjs_wrapper")
set_version("0.1.0")

add_rules("mode.debug", "mode.release")

set_warnings("all")
set_languages("c++17")

add_requires("quickjs-ng",
{
    alias = "quickjs",
    configs = { libc = true, shared = false }
})

target("quickjs_wrapper")
    set_kind("static")
    add_files("src/quickjs/**.cpp")
    add_includedirs("include", {public = true})
    add_includedirs("src", {public = false})
    set_targetdir("lib/$(arch)-$(mode)")

    add_packages("quickjs")
target_end()