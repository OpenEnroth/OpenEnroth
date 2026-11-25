# Regenerates BuildInfo.gen.h on every build invocation (run via a custom
# target with no OUTPUT, which CMake always considers out of date), not just
# on `cmake` reconfigure -- so an incremental `cmake --build` without a
# reconfigure still reports the git revision / build time of the tree that
# was actually compiled.
#
# Expected on the command line (via -D):
#   SOURCE_DIR - repo root, used as the git working directory
#   TEMPLATE   - path to BuildInfo.gen.h.in
#   OUTPUT     - path to write the generated header to

find_package(Git QUIET)

set(MM_GIT_REVISION "unknown")
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE GIT_REVISION_OUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE GIT_RESULT)
    if(GIT_RESULT EQUAL 0 AND NOT "${GIT_REVISION_OUT}" STREQUAL "")
        set(MM_GIT_REVISION "${GIT_REVISION_OUT}")
    endif()
endif()

string(TIMESTAMP MM_BUILD_TIME UTC)

configure_file(${TEMPLATE} ${OUTPUT} @ONLY)
