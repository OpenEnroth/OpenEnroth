function(init_check_tidy)
    if(NOT OE_CHECK_TIDY)
        return()
    endif()

    # clang-tidy reads the compile commands database, and only the ninja and makefile generators write one.
    if(NOT CMAKE_GENERATOR MATCHES "Ninja|Makefiles")
        message(STATUS "The ${CMAKE_GENERATOR} generator exports no compile commands, check_tidy target won't be available")
        return()
    endif()

    find_program(OE_CLANG_TIDY_COMMAND clang-tidy)
    find_program(OE_RUN_CLANG_TIDY_COMMAND NAMES run-clang-tidy run-clang-tidy.py)

    if(NOT OE_CLANG_TIDY_COMMAND OR NOT OE_RUN_CLANG_TIDY_COMMAND)
        message(WARNING "clang-tidy or run-clang-tidy not found, check_tidy target won't be available")
        return()
    endif()

    # clang-tidy discovers gcc toolchains only in the standard prefixes, so when building with a gcc that
    # lives elsewhere it can pair up with an older system libstdc++. Point it at the configured one.
    set(EXTRA_ARGS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -print-libgcc-file-name
                        OUTPUT_VARIABLE LIBGCC_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
        get_filename_component(GCC_INSTALL_DIR "${LIBGCC_PATH}" DIRECTORY)
        if(GCC_INSTALL_DIR)
            set(EXTRA_ARGS "-extra-arg=--gcc-install-dir=${GCC_INSTALL_DIR}")
        endif()
    endif()

    add_custom_target(check_tidy
        # An unknown check name is silently ignored, so a rename upstream would quietly drop an exclusion.
        COMMAND "${OE_CLANG_TIDY_COMMAND}" --verify-config "--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy"
        COMMAND "${OE_RUN_CLANG_TIDY_COMMAND}"
                -clang-tidy-binary "${OE_CLANG_TIDY_COMMAND}"
                -p "${PROJECT_BINARY_DIR}" -quiet
                -config-file "${PROJECT_SOURCE_DIR}/.clang-tidy"
                ${EXTRA_ARGS}
                "${PROJECT_SOURCE_DIR}/src/.*" "${PROJECT_SOURCE_DIR}/test/.*"
        COMMENT "Running clang-tidy"
        USES_TERMINAL # run-clang-tidy parallelizes internally, and its progress output is worth seeing live.
        VERBATIM)
endfunction()
