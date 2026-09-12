import argparse
import json
import os
import re
import subprocess
import sys


def source_pattern(source_dir):
    root = re.escape(os.path.abspath(source_dir))
    separator = re.escape(os.sep)
    return rf"^{root}{separator}(src|test){separator}.*\.(c|cc|cpp|cxx)$"


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("source_dir")
    parser.add_argument("build_dir")
    parser.add_argument("runner")
    parser.add_argument("runner_args", nargs=argparse.REMAINDER)
    args = parser.parse_args(argv)

    pattern = source_pattern(args.source_dir)
    with open(os.path.join(args.build_dir, "compile_commands.json"), encoding="utf-8") as stream:
        database = json.load(stream)
    files = {os.path.abspath(os.path.join(entry["directory"], entry["file"])) for entry in database}
    if not any(re.search(pattern, path) for path in files):
        print("check_tidy: no project C/C++ files selected from compile_commands.json", file=sys.stderr)
        return 1

    command = [sys.executable, args.runner] if args.runner.lower().endswith(".py") else [args.runner]
    return subprocess.run([*command, "-p", args.build_dir, *args.runner_args, pattern]).returncode


if __name__ == "__main__":
    sys.exit(main())
