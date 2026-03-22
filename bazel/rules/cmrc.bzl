"""cmrc_resource_library — Bazel replacement for CMakeRC's cmrc_add_resource_library.

Embeds arbitrary files as C++ char arrays accessible via cmrc::embedded_filesystem.
Symbol naming matches CMakeRC: f_{first4ofMD5(relpath)}_{MAKE_C_IDENTIFIER(relpath)}.
"""

def _relpath(src, strip_prefix):
    """Compute the virtual path for a source file."""
    path = src.short_path
    if strip_prefix:
        prefix = strip_prefix
        if not prefix.endswith("/"):
            prefix = prefix + "/"
        if path.startswith(prefix):
            path = path[len(prefix):]
    return path

def _safe_name(relpath):
    """Turn a relpath into a filename-safe string."""
    result = ""
    for ch in relpath.elems():
        if ch == "/" or ch == "\\" or ch == "." or ch == "-":
            result += "_"
        else:
            result += ch
    return result

def _cmrc_gen_impl(ctx):
    resource_cpps = []
    relpaths = []

    for src in ctx.files.srcs:
        relpath = _relpath(src, ctx.attr.strip_prefix)
        relpaths.append(relpath)

        out_cpp = ctx.actions.declare_file(
            "_cmrc_{}/{}.cpp".format(ctx.label.name, _safe_name(relpath)),
        )

        ctx.actions.run(
            executable = ctx.executable._gen_resource,
            arguments = [
                "--namespace", ctx.attr.namespace,
                "--relpath", relpath,
                "--input", src.path,
                "--output", out_cpp.path,
            ],
            inputs = [src],
            outputs = [out_cpp],
            mnemonic = "CmrcResource",
            progress_message = "Generating cmrc resource {}".format(relpath),
        )
        resource_cpps.append(out_cpp)

    lib_cpp = ctx.actions.declare_file("_cmrc_{}/lib.cpp".format(ctx.label.name))
    ctx.actions.run(
        executable = ctx.executable._gen_lib,
        arguments = [
            "--namespace", ctx.attr.namespace,
            "--output", lib_cpp.path,
            "--relpaths",
        ] + relpaths,
        inputs = [],
        outputs = [lib_cpp],
        mnemonic = "CmrcLib",
        progress_message = "Generating cmrc lib.cpp for {}".format(ctx.label.name),
    )

    return [DefaultInfo(files = depset([lib_cpp] + resource_cpps))]

_cmrc_gen = rule(
    implementation = _cmrc_gen_impl,
    attrs = {
        "namespace": attr.string(mandatory = True),
        "srcs": attr.label_list(allow_files = True),
        "strip_prefix": attr.string(default = ""),
        "_gen_resource": attr.label(
            default = "//bazel/cmrc:generate_resource",
            executable = True,
            cfg = "exec",
        ),
        "_gen_lib": attr.label(
            default = "//bazel/cmrc:generate_lib",
            executable = True,
            cfg = "exec",
        ),
    },
)

def cmrc_resource_library(name, namespace = None, srcs = [], strip_prefix = "", deps = [], visibility = None, **kwargs):
    """Embed resource files into a C++ static library accessible via cmrc::embedded_filesystem.

    Args:
        name: Target name.
        namespace: C++ namespace for get_filesystem() (defaults to name).
        srcs: List of files to embed.
        strip_prefix: Path prefix to strip when computing virtual paths.
        deps: Additional deps for the cc_library.
        visibility: Bazel visibility list.
    """
    if namespace == None:
        namespace = name

    gen_name = name + "_srcs"
    _cmrc_gen(
        name = gen_name,
        namespace = namespace,
        srcs = srcs,
        strip_prefix = strip_prefix,
        visibility = ["//visibility:private"],
    )

    native.cc_library(
        name = name,
        srcs = [":" + gen_name],
        deps = deps + ["//bazel/cmrc:cmrc_base"],
        visibility = visibility,
        **kwargs
    )
