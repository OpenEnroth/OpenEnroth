Read `HACKING.md` before doing any changes in this repo.

Build `check_style` target to check style. Always check style after your changes.

Build `Run_AllTests_Headless_Parallel` target to test your changes. Always run tests after your changes. If you can't find the game data - ask the user to help you locate it, don't just silently skip game tests.

When adding comments, add them in doxygen format. Don't forget to document function arguments and return value. Use `@param[out]` for output parameters.

Use C++23 where appropriate. This means using `contains()` instead of `find() != end()` in most places. Note that even `std::ranges` has `contains()`.

Use `virtual` prefix for all virtual functions, even if clang diagnostics complain about us already having an `override` there. We try to be explicit in most places.

Sort methods in .cpp files in the same order as they appear in the .h file.

In header files, use additional `private:` before listing all class fields at the end of the class declaration.
