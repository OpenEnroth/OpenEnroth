if("${OPENAL_VERSION}" STREQUAL "")
    set(OPENAL_VERSION "1.18.2")
    DEBUG_PRINT("No OPENAL_VERSION specified, defaulting to ${OPENAL_VERSION}")
endif()


set(OPENAL_ARTIFACTS ${CMAKE_CURRENT_BINARY_DIR}/openal-artifacts)
set(OPENAL_INSTALL  ${CMAKE_CURRENT_BINARY_DIR}/openal-install)
set(OPENAL_SRC      ${OPENAL_INSTALL}/src/openal_builder)
set(OPENAL_INCLUDE  ${OPENAL_INSTALL}/src/openal_builder/include)
set(OPENAL_BIN      ${OPENAL_INSTALL}/src/openal_builder/bin/Win32)
set(OPENAL_LIB      ${OPENAL_INSTALL}/src/openal_builder/libs/Win32)
set(OPENAL_SHARE    ${OPENAL_INSTALL}/share)
DEBUG_PRINT("OPENAL_ARTIFACTS ${OPENAL_ARTIFACTS}")
DEBUG_PRINT("OPENAL_INSTALL   ${OPENAL_INSTALL}")
DEBUG_PRINT("OPENAL_SRC       ${OPENAL_SRC}")
DEBUG_PRINT("OPENAL_INCLUDE   ${OPENAL_INCLUDE}")
DEBUG_PRINT("OPENAL_BIN       ${OPENAL_BIN}")
DEBUG_PRINT("OPENAL_LIB       ${OPENAL_LIB}")
DEBUG_PRINT("OPENAL_SHARE     ${OPENAL_SHARE}")


add_library(openal STATIC IMPORTED)
add_dependencies(openal openal_builder)
include_directories(${OPENAL_INCLUDE})
    
set_property(TARGET openal PROPERTY IMPORTED_LOCATION "${OPENAL_LIB}/OpenAL32.lib")

list(APPEND DEPENDENCIES "${OPENAL_BIN}/OpenAL32.dll")

NINJA_WORKAROUND_GET_BYPRODUCTS(openal)



# For win32 it's faster to download the pre-compiled development binaries.
if (WIN32)
    ExternalProject_Add(
        openal_builder
        PREFIX "${OPENAL_INSTALL}"
        DOWNLOAD_DIR "${OPENAL_ARTIFACTS}"
        URL https://openal-soft.org/openal-binaries/openal-soft-${OPENAL_VERSION}-bin.zip
        URL_MD5 03cf7160ca84c36c2e6560fb278308fc
        INSTALL_DIR "${OPENAL_INSTALL}"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
#            COMMAND ${CMAKE_COMMAND} -E copy ${OPENAL_INSTALL}/src/openalsoft/bin/Win32/soft_oal.dll  ${DEPENDENCY_INSTALL_DIR}/bin/OpenAL32.dll     
#            COMMAND ${CMAKE_COMMAND} -E copy_directory ${OPENAL_INSTALL}/src/openalsoft/libs/Win32    ${DEPENDENCY_INSTALL_DIR}/lib
#            COMMAND ${CMAKE_COMMAND} -E copy_directory ${OPENAL_INSTALL}/src/openalsoft/include/AL    ${DEPENDENCY_INSTALL_DIR}/include/AL

        BUILD_BYPRODUCTS  ${OPENAL_BYPRODUCTS}
    )

    add_custom_command(
        TARGET openal_builder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${OPENAL_BIN}/soft_oal.dll" "${OPENAL_BIN}/OpenAL32.dll"
    )

 # build from source
else()
  message(FATAL_ERROR "OpenAL: No non-win builds yet" )

#    SET(OPENALSOFT_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/openal)
#
#    SET(OPENALSOFT_CMAKE_ARGS ${COMMON_CMAKE_ARGS}
#        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
#        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> 
#        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
#        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
#        -DCMAKE_DEBUG_POSTFIX=d
#        -DALSOFT_CONFIG=OFF
#        -DALSOFT_EXAMPLES=OFF
#        -DALSOFT_NO_CONFIG_UTIL=ON
#        -DALSOFT_UTILS=OFF
#        #-DLIBTYPE=STATIC
#    )
#
#    ExternalProject_Add(
#        openalsoft
#        PREFIX ${DEPENDENCY_EXTRACT_DIR}
#        DOWNLOAD_DIR ${DEPENDENCY_DOWNLOAD_DIR}
#        URL http://kcat.strangesoft.net/openal-releases/openal-soft-${OPENAL_VERSION}.tar.bz2
#        URL_MD5 d4eeb0889812e2fdeaa1843523d76190
#        INSTALL_DIR ${DEPENDENCY_INSTALL_DIR}
#        CMAKE_ARGS ${OPENALSOFT_CMAKE_ARGS}
#    )
endif()