# Core imgui library (no backend).
cc_library(
    name = "imgui_core",
    srcs = [
        "imgui.cpp",
        "imgui_demo.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
    ],
    hdrs = [
        "imconfig.h",
        "imgui.h",
        "imgui_internal.h",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
)

# SDL3 + OpenGL3 backend — the combination used by OpenEnroth.
cc_library(
    name = "imgui",
    srcs = [
        "backends/imgui_impl_sdl3.cpp",
        "backends/imgui_impl_opengl3.cpp",
    ],
    hdrs = [
        "backends/imgui_impl_sdl3.h",
        "backends/imgui_impl_opengl3.h",
        "backends/imgui_impl_opengl3_loader.h",
    ],
    includes = ["backends"],
    deps = [
        ":imgui_core",
        "//bazel/system:sdl3",
        "//bazel/system:opengl",
        "//thirdparty/glad:glad",
    ],
    visibility = ["//visibility:public"],
)
