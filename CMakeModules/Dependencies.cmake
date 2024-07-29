
function(download_prebuilt_dependencies TAG FILE_NAME TARGET_DIR OUT_STATUS_VAR)
    set(SOURCE_URL "https://github.com/OpenEnroth/OpenEnroth_Dependencies/releases/download/${TAG}/${FILE_NAME}")
    set(TARGET_PATH "${TARGET_DIR}/${FILE_NAME}")
    message(STATUS "Downloading ${SOURCE_URL} to ${TARGET_PATH}...")
    file(DOWNLOAD
            "${SOURCE_URL}"
            "${TARGET_PATH}"
            SHOW_PROGRESS
            STATUS DOWNLOAD_STATUS
            TIMEOUT 60)  # seconds

    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
    if(STATUS_CODE EQUAL 0)
        message(STATUS "Downloaded ${SOURCE_URL}!")
    else()
        file(REMOVE "${TARGET_PATH}")
        message(WARNING "Could not download ${SOURCE_URL}: ${ERROR_MESSAGE}")
        set(${OUT_STATUS_VAR} 1 PARENT_SCOPE)
        return()
    endif()

    message(STATUS "Extracting ${TARGET_PATH} to ${TARGET_DIR}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${TARGET_PATH}"
            WORKING_DIRECTORY ${TARGET_DIR}
            RESULT_VARIABLE UNPACK_STATUS
            COMMAND_ECHO STDOUT)

    if (UNPACK_STATUS)
        message(WARNING "Could not unpack ${TARGET_PATH}: ${UNPACK_STATUS}")
        set(${OUT_STATUS_VAR} 1 PARENT_SCOPE)
        return()
    endif()

    set(${OUT_STATUS_VAR} 0 PARENT_SCOPE)
endfunction()

function(print_library_found_message LIBRARY TARGET_NAME INCLUDE_DIR)
    if(NOT "${TARGET_NAME}" STREQUAL "")
        get_target_property(TARGET_INCLUDE_DIR "${TARGET_NAME}" INTERFACE_INCLUDE_DIRECTORIES)
    endif()
    if("${TARGET_INCLUDE_DIR}" STREQUAL "")
        set(TARGET_INCLUDE_DIR "${INCLUDE_DIR}")
    endif()
    if(NOT "${TARGET_INCLUDE_DIR}" STREQUAL "")
        message(STATUS "${LIBRARY} found: ${TARGET_INCLUDE_DIR}")
    endif()
endfunction()

macro(resolve_dependencies) # Intentionally a macro - we want set() to work in parent scope.
    if(OE_USE_PREBUILT_DEPENDENCIES)
        # "r3" is version as set in yml files in OpenEnroth_Dependencies, "master" is a branch name. This way it's
        # possible to test with dependencies built from different branches of the OpenEnroth_Dependencies repo.
        set(PREBUILT_DEPS_TAG "deps_r3_master")
        message(STATUS "Using prebuilt dependencies with PREBUILT_DEPS_TAG=${PREBUILT_DEPS_TAG}")

        set(PREBUILT_DEPS_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
        if (PREBUILT_DEPS_BUILD_TYPE STREQUAL "RelWithDebInfo" OR PREBUILT_DEPS_BUILD_TYPE STREQUAL "MinSizeRel")
            set(PREBUILT_DEPS_BUILD_TYPE "Release")
        endif()

        set(PREBUILT_DEPS_FILENAME "${OE_BUILD_PLATFORM}_${PREBUILT_DEPS_BUILD_TYPE}_${OE_BUILD_ARCHITECTURE}.zip")
        set(PREBUILT_DEPS_DIR "${CMAKE_CURRENT_BINARY_DIR}/dependencies")
        if (NOT EXISTS "${PREBUILT_DEPS_DIR}/${PREBUILT_DEPS_FILENAME}")
            download_prebuilt_dependencies("${PREBUILT_DEPS_TAG}" "${PREBUILT_DEPS_FILENAME}" "${PREBUILT_DEPS_DIR}" DOWNLOAD_STATUS)
            if(NOT DOWNLOAD_STATUS EQUAL 0)
                message(FATAL_ERROR "Downloading prebuilt dependencies failed, consider rerunning cmake with -DOE_USE_PREBUILT_DEPENDENCIES=OFF.")
            endif()
        endif()

        # Android sets CMAKE_FIND_ROOT_PATH and this breaks find_package for us. So we hack.
        if(CMAKE_FIND_ROOT_PATH)
            list(APPEND CMAKE_FIND_ROOT_PATH "${PREBUILT_DEPS_DIR}")
            list(APPEND CMAKE_MODULE_PATH "/")
            list(APPEND CMAKE_PREFIX_PATH "/")
        else()
            list(APPEND CMAKE_MODULE_PATH "${PREBUILT_DEPS_DIR}")
            list(APPEND CMAKE_PREFIX_PATH "${PREBUILT_DEPS_DIR}")
        endif()

        # Prebuilt zlib is static, so we instruct the find_package to look for the static one.
        set(ZLIB_USE_STATIC_LIBS ON)
    else()
        message(STATUS "Not using prebuilt dependencies")
    endif()

    if(OE_USE_DUMMY_DEPENDENCIES)
        # Just create dummy libs so that configure pass works. We won't be building anything.
        add_library(ZLIB INTERFACE)
        add_library(ZLIB::ZLIB ALIAS ZLIB)
        add_library(OpenAL INTERFACE)
        add_library(OpenAL::OpenAL ALIAS OpenAL)
        add_library(SDL2OE INTERFACE)
        add_library(SDL2::SDL2 ALIAS SDL2OE)
        add_library(SDL2::SDL2OE ALIAS SDL2OE)
        set(SDL2_FOUND ON)
    else()
        # Prebuilt & user-supplied deps are resolved using the same code here.
        find_package(ZLIB REQUIRED)
        find_package(FFmpeg REQUIRED)

        find_package(SDL2 CONFIG REQUIRED)
        add_library(SDL2OE INTERFACE)
        target_link_libraries(SDL2OE INTERFACE SDL2::SDL2)
        if(TARGET SDL2::SDL2main) # Not all platforms have SDL2main.
            target_link_libraries(SDL2OE INTERFACE SDL2::SDL2main)
        endif()
        add_library(SDL2::SDL2OE ALIAS SDL2OE)

        # This should find OpenALConfig.cmake that comes with OpenAL Soft.
        #
        # Not even trying to do this w/o CONFIG because on MacOS it will find a MacOS framework, and there are two
        # problems with it:
        # - `FindOpenAL.cmake` sets the link target to point to the framework dir, which obviously doesn't work. We
        #   should be linking to `Frameworks/OpenAL.framework/OpenAL.tbd`.
        # - OpenEnroth doesn't support OpenAL that ships with MacOS. As in, sound barely works.
        #
        # If you're getting an error here, try passing something like -DOPENAL_ROOT=/opt/homebrew/opt/openal-soft to cmake.
        find_package(OpenAL CONFIG REQUIRED)
    endif()

    # On Android we somehow get OpenGL available by default, despite it not being findable by find_package. So we
    # just create a dummy lib.
    if(OE_USE_DUMMY_DEPENDENCIES OR OE_BUILD_PLATFORM STREQUAL "android")
        add_library(OpenGL_GL INTERFACE)
        add_library(OpenGL::GL ALIAS OpenGL_GL)
    else()
        find_package(OpenGL REQUIRED)
    endif()

    print_library_found_message(OpenAL OpenAL::OpenAL "")
    print_library_found_message(ZLIB ZLIB::ZLIB "")
    print_library_found_message(FFmpeg "" "${AVCODEC_INCLUDE_DIRS}")
    print_library_found_message(SDL2 SDL2::SDL2 "")
    print_library_found_message(OpenGL OpenGL::GL "")
endmacro()
