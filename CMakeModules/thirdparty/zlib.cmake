# hack to be able to create a convenience library without sources
file(WRITE ${LIB_DIR}/zlib.c "")

add_library(
    zlib STATIC
    ${LIB_DIR}/zlib.c
)

if (WIN32)
set(ZLIB_DIR "${LIBRARY_DIR}/zlib-1.2.11")
set(ZLIB_BIN_DIR "${ZLIB_DIR}/bin")
set(ZLIB_LIB_DIR "${ZLIB_DIR}/lib")
set(ZLIB_INCLUDE_DIR "${ZLIB_DIR}/include")

set(ZLIB_DEBUG_APPENDIX "")
if (NOT (CMAKE_BUILD_TYPE MATCHES "RELEASE"))
    set(ZLIB_DEBUG_APPENDIX "d")
endif()

set_target_properties(
    zlib PROPERTIES
    PUBLIC_HEADER "${ZLIB_INCLUDE_DIR}/zlib.h"
)

target_link_libraries(
    zlib
    "${ZLIB_LIB_DIR}/zlibstatic${ZLIB_DEBUG_APPENDIX}.lib"
)

include_directories(
    "${ZLIB_INCLUDE_DIR}"
)

else()  # WIN32
find_package(ZLIB REQUIRED)
target_link_libraries(zlib z)
endif()
