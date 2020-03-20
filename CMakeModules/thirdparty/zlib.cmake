if("${ZLIB_VERSION}" STREQUAL "")
    set(ZLIB_VERSION "1.2.11")
    DEBUG_PRINT("No ZLIB_VERSION specified, defaulting to ${ZLIB_VERSION}")
endif()

set(ZLIB_URL ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zlib-${ZLIB_VERSION}.tar.gz)
if (NOT EXISTS ${ZLIB_URL})
    set (ZLIB_URL "--internet url--")
endif()

set(ZLIB_INSTALL ${CMAKE_CURRENT_BINARY_DIR}/zlib-install)
set(ZLIB_SRC ${CMAKE_CURRENT_BINARY_DIR}/zlib-src)
set(ZLIB_INCLUDE ${ZLIB_INSTALL}/include)
set(ZLIB_BIN ${ZLIB_INSTALL}/bin)
set(ZLIB_LIB ${ZLIB_INSTALL}/lib)
set(ZLIB_SHARE ${ZLIB_INSTALL}/share)
DEBUG_PRINT("ZLIB_INSTALL   ${ZLIB_INSTALL}")
DEBUG_PRINT("ZLIB_SRC       ${ZLIB_SRC}")
DEBUG_PRINT("ZLIB_INCLUDE   ${ZLIB_INCLUDE}")
DEBUG_PRINT("ZLIB_BIN       ${ZLIB_BIN}")
DEBUG_PRINT("ZLIB_LIB       ${ZLIB_LIB}")
DEBUG_PRINT("ZLIB_SHARE     ${ZLIB_SHARE}")






    add_library(zlib STATIC IMPORTED)
    add_dependencies(zlib zlib_builder)
    include_directories(${ZLIB_INCLUDE})
    
    set_property(TARGET zlib PROPERTY IMPORTED_LOCATION "${ZLIB_LIB}/zlibd.lib")
    set_property(TARGET zlib PROPERTY IMPORTED_LOCATION_DEBUG "${ZLIB_LIB}/zlibd.lib")
    set_property(TARGET zlib PROPERTY IMPORTED_LOCATION_RELEASE "${ZLIB_LIB}/zlibd.lib")

    NINJA_WORKAROUND_GET_BYPRODUCTS(zlib)

  






ExternalProject_Add(zlib_builder
    GIT_REPOSITORY "https://github.com/madler/zlib.git"
    GIT_TAG "v${ZLIB_VERSION}"

    PREFIX "zlib"
    SOURCE_DIR "${ZLIB_SRC}"
    CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${ZLIB_INSTALL}

    BUILD_BYPRODUCTS  ${ZLIB_BYPRODUCTS}
)



list(APPEND DEPENDENCIES "${ZLIB_BIN}/zlibd.dll")