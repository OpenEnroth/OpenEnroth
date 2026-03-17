"""Generate a single cmrc resource .cpp from an input file.

Usage:
    python generate_resource.py --namespace NS --relpath REL --input PATH --output PATH

Symbol naming matches CMakeRC: f_{first4ofMD5(relpath)}_{MAKE_C_IDENTIFIER(relpath)}
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


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--namespace", required=True)
    p.add_argument("--relpath", required=True)
    p.add_argument("--input", required=True)
    p.add_argument("--output", required=True)
    args = p.parse_args()

    sym = encode_fpath(args.relpath)

    with open(args.input, "rb") as f:
        data = f.read()

    # Format as hex char literals
    chars = ["'\\x{:02x}'".format(b) for b in data] + ["0"]
    chars_str = ",".join(chars)

    cpp = (
        "namespace {{ const char file_array[] = {{ {chars} }}; }}\n"
        "namespace cmrc {{ namespace {ns} {{ namespace res_chars {{\n"
        "extern const char* const {sym}_begin = file_array;\n"
        "extern const char* const {sym}_end = file_array + {n};\n"
        "}}}}}}\n"
    ).format(chars=chars_str, ns=args.namespace, sym=sym, n=len(data))

    with open(args.output, "w") as f:
        f.write(cpp)


if __name__ == "__main__":
    main()
