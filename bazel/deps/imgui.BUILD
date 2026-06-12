# Core imgui library (no backend).
# include_prefix = "imgui" makes headers accessible as <imgui/imgui.h>, etc.
# includes = ["."] keeps direct "imgui.h" working for imgui's own source files.
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
    include_prefix = "imgui",
    visibility = ["//visibility:public"],
)

# Patch backend headers: replace #include "imgui.h" with #include <imgui/imgui.h>.
# This mirrors what CMake does in thirdparty/imgui/CMakeLists.txt via copy_and_patch_imgui_file().
# Without patching, backend headers include "imgui.h" via includes=["."] while callers
# include <imgui/imgui.h> via include_prefix, causing ODR violations (two physical copies).
genrule(
    name = "imgui_backends_patched",
    srcs = [
        "backends/imgui_impl_sdl3.h",
        "backends/imgui_impl_opengl3.h",
        "backends/imgui_impl_opengl3_loader.h",
    ],
    outs = [
        "patched/backends/imgui_impl_sdl3.h",
        "patched/backends/imgui_impl_opengl3.h",
        "patched/backends/imgui_impl_opengl3_loader.h",
    ],
    cmd = """
        for src in $(SRCS); do
            base=$$(basename $$src)
            dst=$(RULEDIR)/patched/backends/$$base
            sed 's|#include "imgui.h"|#include <imgui/imgui.h>|g' $$src > $$dst
        done
    """,
)

genrule(
    name = "imgui_backends_srcs_patched",
    srcs = [
        "backends/imgui_impl_sdl3.cpp",
        "backends/imgui_impl_opengl3.cpp",
    ],
    outs = [
        "patched/backends/imgui_impl_sdl3.cpp",
        "patched/backends/imgui_impl_opengl3.cpp",
    ],
    cmd = """
        for src in $(SRCS); do
            base=$$(basename $$src)
            dst=$(RULEDIR)/patched/backends/$$base
            sed 's|#include "imgui.h"|#include <imgui/imgui.h>|g;s|#include <GLES3/gl3.h>|#include <glad/gl.h>|g' $$src > $$dst
        done
    """,
)

# SDL3 + OpenGL3 backend — the combination used by OpenEnroth.
# include_prefix = "imgui" makes headers accessible as <imgui/backends/imgui_impl_opengl3.h>, etc.
cc_library(
    name = "imgui",
    srcs = [":imgui_backends_srcs_patched"],
    hdrs = [":imgui_backends_patched"],
    include_prefix = "imgui",
    strip_include_prefix = "patched",
    deps = [
        ":imgui_core",
        "@@//bazel/system:sdl3",
        "@@//bazel/system:opengl",
        "@@//thirdparty/glad:glad",
    ],
    visibility = ["//visibility:public"],
)
