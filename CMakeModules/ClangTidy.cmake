function(init_check_tidy)
    find_program(OE_CLANG_TIDY_COMMAND clang-tidy)
    find_program(OE_RUN_CLANG_TIDY_COMMAND NAMES run-clang-tidy run-clang-tidy.py)

    if(NOT OE_CLANG_TIDY_COMMAND OR NOT OE_RUN_CLANG_TIDY_COMMAND)
        message(STATUS "clang-tidy not found, check_tidy target won't be available")
        return()
    endif()

    # Checks are configured in .clang-tidy at the repo root.
    add_custom_target(check_tidy
        COMMAND "${OE_RUN_CLANG_TIDY_COMMAND}"
                -clang-tidy-binary "${OE_CLANG_TIDY_COMMAND}"
                -p "${PROJECT_BINARY_DIR}" -quiet
                -config-file "${PROJECT_SOURCE_DIR}/.clang-tidy"
                "${PROJECT_SOURCE_DIR}/src/.*" "${PROJECT_SOURCE_DIR}/test/.*"
        COMMENT "Running clang-tidy"
        VERBATIM)

    # Scripting sources include the luajit.h that's generated when luajit is built.
    add_dependencies(check_tidy libluajit)
endfunction()
