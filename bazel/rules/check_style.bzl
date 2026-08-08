"""cpplint over first-party cc targets, as cached per-target bazel actions."""

_EXTENSIONS = ("c", "cc", "cpp", "cxx", "h", "hh")

_CheckStyleInfo = provider(
    doc = "Lint results.",
    fields = {"markers": "depset of per-target lint markers"},
)

def _lintable(label):
    return label.workspace_name == "" and (label.package.startswith("src") or label.package.startswith("test"))

def _check_style_aspect_impl(target, ctx):
    transitive = [
        dep[_CheckStyleInfo].markers
        for dep in getattr(ctx.rule.attr, "deps", [])
        if _CheckStyleInfo in dep
    ]
    markers = []
    if _lintable(target.label):
        files = [
            f
            for attr in ("srcs", "hdrs")
            for t in getattr(ctx.rule.attr, attr, [])
            for f in t.files.to_list()
            if f.is_source and f.extension in _EXTENSIONS
        ]
        if files:
            marker = ctx.actions.declare_file(target.label.name + ".check_style")
            args = ctx.actions.args()
            args.add(ctx.executable._cpplint)
            args.add("--quiet")
            args.add_all(files)
            ctx.actions.run_shell(
                command = '"$@" && touch ' + marker.path,
                arguments = [args],
                inputs = files + ctx.files._config,
                outputs = [marker],
                tools = [ctx.executable._cpplint],
                mnemonic = "CppLint",
                progress_message = "Linting %{label}",
            )
            markers.append(marker)
    return [_CheckStyleInfo(markers = depset(markers, transitive = transitive))]

_check_style_aspect = aspect(
    implementation = _check_style_aspect_impl,
    attr_aspects = ["deps"],
    attrs = {
        "_config": attr.label(default = "//:cpplint_config", allow_files = True),
        "_cpplint": attr.label(
            default = "@cpplint//:cpplint",
            cfg = "exec",
            executable = True,
        ),
    },
)

def _check_style_impl(ctx):
    return DefaultInfo(files = depset(transitive = [
        dep[_CheckStyleInfo].markers
        for dep in ctx.attr.deps
    ] + [
        check.files
        for check in ctx.attr.extra_checks
    ]))

check_style = rule(
    implementation = _check_style_impl,
    attrs = {
        "deps": attr.label_list(aspects = [_check_style_aspect]),
        "extra_checks": attr.label_list(
            doc = "Checks that are not cpplint-over-cc-targets, e.g. the lua lint marker.",
            allow_files = True,
        ),
    },
)
