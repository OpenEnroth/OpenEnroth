# Assembles a macOS .app bundle the way cmake's MACOSX_BUNDLE does - a plain
# directory tree with a rendered Info.plist, PkgInfo and Resources. No signing
# step, matching cmake (the linker's ad-hoc signature is enough for local use,
# releases sign out-of-band).

def _macos_app_impl(ctx):
    app = ctx.actions.declare_directory(ctx.attr.bundle_name + ".app")

    # The same substitutions cmake's configure step applies to Info.plist.in,
    # with the version freshly stamped from stable-status.txt.
    substitutions = {
        "MACOSX_BUNDLE_EXECUTABLE_NAME": ctx.attr.bundle_name,
        "MACOSX_BUNDLE_BUNDLE_NAME": ctx.attr.bundle_name,
        "MACOSX_BUNDLE_GUI_IDENTIFIER": ctx.attr.bundle_name,
        "MACOSX_BUNDLE_ICON_FILE": ctx.attr.bundle_name,
        "MACOSX_BUNDLE_COPYRIGHT": ctx.attr.copyright,
        "MACOSX_BUNDLE_INFO_STRING": ctx.attr.info_string,
    }
    sed_args = "".join([" -e 's|${%s}|%s|g'" % (key, value) for key, value in substitutions.items()])
    version_keys = [
        "MACOSX_BUNDLE_BUNDLE_VERSION",
        "MACOSX_BUNDLE_LONG_VERSION_STRING",
        "MACOSX_BUNDLE_SHORT_VERSION_STRING",
    ]
    sed_args += "".join([" -e \"s|\\${%s}|$ver|g\"" % key for key in version_keys])

    command = "\n".join([
        "set -e",
        "ver=$(sed -n 's/^STABLE_OE_VERSION //p' '%s')" % ctx.info_file.path,
        "ver=${ver:-unknown}",
        "mkdir -p '%s/Contents/MacOS' '%s/Contents/Resources'" % (app.path, app.path),
        "sed%s '%s' > '%s/Contents/Info.plist'" % (sed_args, ctx.file.infoplist.path, app.path),
        "printf 'APPL????' > '%s/Contents/PkgInfo'" % app.path,
        "cp '%s' '%s/Contents/MacOS/%s'" % (ctx.executable.binary.path, app.path, ctx.attr.bundle_name),
    ] + [
        "cp '%s' '%s/Contents/Resources/'" % (resource.path, app.path)
        for resource in ctx.files.resources
    ])

    ctx.actions.run_shell(
        inputs = [ctx.executable.binary, ctx.file.infoplist, ctx.info_file] + ctx.files.resources,
        outputs = [app],
        command = command,
        mnemonic = "MacosApp",
        progress_message = "Bundling %s.app" % ctx.attr.bundle_name,
    )
    return [DefaultInfo(files = depset([app]))]

macos_app = rule(
    implementation = _macos_app_impl,
    attrs = {
        "binary": attr.label(mandatory = True, executable = True, cfg = "target"),
        "infoplist": attr.label(mandatory = True, allow_single_file = True),
        "resources": attr.label_list(allow_files = True),
        "bundle_name": attr.string(mandatory = True),
        "copyright": attr.string(default = ""),
        "info_string": attr.string(default = ""),
    },
)
