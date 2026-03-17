"""Generate the cmrc library index .cpp for a resource library.

Usage:
    python generate_lib.py --namespace NS --output PATH --relpaths rel1 rel2 ...

Computes symbol names from relpaths using the same scheme as CMakeRC:
    f_{first4ofMD5(relpath)}_{MAKE_C_IDENTIFIER(relpath)}
"""

import argparse
import hashlib
import re


def make_c_identifier(s):
    ident = re.sub(r"[^a-zA-Z0-9_]", "_", s)
    if ident and ident[0].isdigit():
        ident = "_" + ident
    return ident


def encode_fpath(relpath):
    md5 = hashlib.md5(relpath.encode()).hexdigest()
    return "f_{}_{}".format(md5[:4], make_c_identifier(relpath))


def dir_sym(dirpath):
    """Variable-name-safe identifier for a directory path."""
    return "d_" + re.sub(r"[^a-zA-Z0-9]", "_", dirpath)


def all_parent_dirs(relpaths):
    """Return sorted list of all unique non-empty directory components."""
    dirs = set()
    for p in relpaths:
        parts = p.split("/")
        for i in range(1, len(parts)):
            dirs.add("/".join(parts[:i]))
    return sorted(dirs)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--namespace", required=True)
    p.add_argument("--output", required=True)
    p.add_argument("--relpaths", nargs="*", default=[])
    args = p.parse_args()

    relpaths = args.relpaths
    dirs = all_parent_dirs(relpaths)

    extern_lines = []
    for relpath in relpaths:
        sym = encode_fpath(relpath)
        extern_lines.append("// {}\n".format(relpath))
        extern_lines.append("extern const char* const {}_begin;\n".format(sym))
        extern_lines.append("extern const char* const {}_end;\n".format(sym))

    index_lines = []
    index_lines.append("namespace {\n\n")
    index_lines.append("const cmrc::detail::index_type&\n")
    index_lines.append("get_root_index() {\n")
    index_lines.append("    static cmrc::detail::directory root_directory_;\n")
    index_lines.append("    static cmrc::detail::file_or_directory root_directory_fod{root_directory_};\n")
    index_lines.append("    static cmrc::detail::index_type root_index;\n")
    index_lines.append("    root_index.emplace(\"\", &root_directory_fod);\n")
    index_lines.append("    struct dir_inl { class cmrc::detail::directory& directory; };\n")
    index_lines.append("    dir_inl root_directory_dir{root_directory_};\n")
    index_lines.append("    (void)root_directory_dir;\n")

    for d in dirs:
        parent = d.rsplit("/", 1)[0] if "/" in d else ""
        leaf = d.rsplit("/", 1)[-1]
        parent_var = "root_directory" if parent == "" else dir_sym(parent)
        sym = dir_sym(d)
        index_lines.append(
            "    static auto {sym}_dir = {parent}_dir.directory.add_subdir(\"{leaf}\");\n".format(
                sym=sym, parent=parent_var, leaf=leaf
            )
        )
        index_lines.append(
            "    root_index.emplace(\"{d}\", &{sym}_dir.index_entry);\n".format(
                d=d, sym=sym
            )
        )

    for relpath in relpaths:
        sym = encode_fpath(relpath)
        dirpath = relpath.rsplit("/", 1)[0] if "/" in relpath else ""
        leaf = relpath.rsplit("/", 1)[-1]
        parent_var = "root_directory" if dirpath == "" else dir_sym(dirpath)
        index_lines.append(
            "    root_index.emplace(\"{relpath}\", {parent}_dir.directory.add_file(\"{leaf}\", res_chars::{sym}_begin, res_chars::{sym}_end));\n".format(
                relpath=relpath, parent=parent_var, leaf=leaf, sym=sym
            )
        )

    index_lines.append("    return root_index;\n")
    index_lines.append("}\n\n")
    index_lines.append("} // anonymous\n\n")

    cpp = (
        "#include <cmrc/cmrc.hpp>\n"
        "#include <map>\n"
        "#include <utility>\n\n"
        "namespace cmrc {{\n"
        "namespace {ns} {{\n\n"
        "namespace res_chars {{\n"
        "{externs}"
        "}} // res_chars\n\n"
        "{index}"
        "cmrc::embedded_filesystem get_filesystem() {{\n"
        "    static auto& index = get_root_index();\n"
        "    return cmrc::embedded_filesystem{{index}};\n"
        "}}\n\n"
        "}} // {ns}\n"
        "}} // cmrc\n"
    ).format(
        ns=args.namespace,
        externs="".join(extern_lines),
        index="".join(index_lines),
    )

    with open(args.output, "w") as f:
        f.write(cpp)


if __name__ == "__main__":
    main()
