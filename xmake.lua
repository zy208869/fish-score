add_rules("mode.debug", "mode.release")
set_languages("c++20")

-- LeviLeviMina SDK
add_repositories("lm-repo https://github.com/Engine-Prime/LeviLeviMina.git")
add_requires("levilamina 0.19.0-cn.7")

-- nlohmann_json for JSON parsing
add_requires("nlohmann_json")

-- cpr for HTTP requests (C++ Requests)
add_requires("cpr")

target("fish-score")
    set_kind("shared")
    add_packages("levilamina", "nlohmann_json", "cpr")

    -- Include dirs
    add_includedirs("include")

    -- Source files
    add_files("src/**.cpp")

    -- Plugin info
    set_license("MIT")
    set_author("YourName")
    set_version("1.0.0")

    -- Copy plugin DLL to BDS plugins folder (optional)
    after_build(function (target)
        import("core.base.task")
        -- 可以取消注释下面这行来自动复制到 BDS 插件目录
        -- os.cp(target:targetfile(), "path/to/bds/plugins/")
    end)
