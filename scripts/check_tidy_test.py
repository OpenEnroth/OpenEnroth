import contextlib
import io
import json
import os
import re
import sys
import tempfile
import unittest
from unittest import mock

import check_tidy


class CheckTidyTest(unittest.TestCase):
    def test_source_selection(self):
        # Windows-normalized paths and regex metacharacters in the checkout path selected no source files.
        root = os.path.abspath(os.path.join("work space", "OpenEnroth [test]+(1).git"))
        pattern = check_tidy.source_pattern(root)
        for folder in ("src", "test"):
            for extension in ("c", "cc", "cpp", "cxx"):
                with self.subTest(folder=folder, extension=extension):
                    self.assertIsNotNone(re.search(pattern, os.path.join(root, folder, "nested", f"file.{extension}")))

        for path in (
            os.path.join(root, "thirdparty", "src", "file.cpp"),
            os.path.join(root, "src", "file.rc"),
            os.path.join(root, "src", "file.h"),
            os.path.join(root, "src", "file.cpp.bak"),
            os.path.join(root + "-other", "src", "file.cpp"),
        ):
            with self.subTest(path=path):
                self.assertIsNone(re.search(pattern, path))

    def test_empty_selection_fails_without_running_tidy(self):
        # run-clang-tidy returned success when the file filter matched nothing.
        with tempfile.TemporaryDirectory() as root:
            database_path = os.path.join(root, "compile_commands.json")
            for database in ([], [{"directory": root, "file": "thirdparty/library.cpp"}], [{"directory": root, "file": "src/resource.rc"}]):
                with self.subTest(database=database):
                    with open(database_path, "w", encoding="utf-8") as stream:
                        json.dump(database, stream)
                    with mock.patch("check_tidy.subprocess.run") as run, contextlib.redirect_stderr(io.StringIO()) as error:
                        result = check_tidy.main([root, root, "run-clang-tidy"])
                    run.assert_not_called()
                    self.assertEqual(result, 1)
                    self.assertIn("no project C/C++ files selected", error.getvalue())

    def test_relative_entry_and_runner_exit(self):
        # The Windows file filter excluded project source files from the compilation database.
        with tempfile.TemporaryDirectory(prefix="tidy [test] ") as root:
            with open(os.path.join(root, "compile_commands.json"), "w", encoding="utf-8") as stream:
                json.dump([{"directory": os.path.join(root, "build"), "file": "../src/file.cpp"}], stream)
            for runner, command in (("run-clang-tidy", ["run-clang-tidy"]), ("run-clang-tidy.py", [sys.executable, "run-clang-tidy.py"])):
                for code in (0, 7):
                    with self.subTest(runner=runner, code=code), mock.patch("check_tidy.subprocess.run") as run:
                        run.return_value.returncode = code
                        self.assertEqual(check_tidy.main([root, root, runner, "-quiet", "-extra-arg=test"]), code)
                        run.assert_called_once_with([
                            *command, "-p", root, "-quiet", "-extra-arg=test", check_tidy.source_pattern(root)
                        ])

    def test_missing_or_invalid_database_does_not_run_tidy(self):
        # The lint target could report success without checking project files.
        with tempfile.TemporaryDirectory() as root, mock.patch("check_tidy.subprocess.run") as run:
            with self.assertRaises(FileNotFoundError):
                check_tidy.main([root, root, "run-clang-tidy"])
            with open(os.path.join(root, "compile_commands.json"), "w", encoding="utf-8") as stream:
                stream.write("not json")
            with self.assertRaises(json.JSONDecodeError):
                check_tidy.main([root, root, "run-clang-tidy"])
            run.assert_not_called()


if __name__ == "__main__":
    unittest.main()
