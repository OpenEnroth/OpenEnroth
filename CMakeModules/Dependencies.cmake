
function(download_prebuilt_dependencies TAG FILE_NAME TARGET_DIR)
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
    if(${STATUS_CODE} EQUAL 0)
        message(STATUS "Downloaded ${SOURCE_URL}!")
    else()
        file(REMOVE "${TARGET_PATH}")
        message(FATAL_ERROR "Could not download ${SOURCE_URL}: ${ERROR_MESSAGE}")
    endif()

    message(STATUS "Extracting ${TARGET_PATH} to ${TARGET_DIR}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${TARGET_PATH}"
            WORKING_DIRECTORY ${TARGET_DIR}
            RESULT_VARIABLE UNPACK_STATUS
            COMMAND_ECHO STDOUT)

    if (UNPACK_STATUS)
        message(FATAL_ERROR "Could not unpack ${TARGET_PATH}: ${UNPACK_STATUS}")
    endif()
endfunction()

#TODO: all prebuilt dependency artifacts should be built and packaged in the same unified way.
#      so all code below could be drastically simplified and we wouldn't have per-platform blocks.

macro(resolve_dependencies) # Intentionally a macro - we want set() to work in parent scope.
    if(OE_USE_PREBUILT_DEPENDENCIES)
        # "r2" is version as set in yml files in OpenEnroth_Dependencies, "master" is a branch name. This way it's
        # possible to test with dependencies built from different branches of the OpenEnroth_Dependencies repo.
        set(PREBUILT_DEPS_TAG "deps_r2_master")
        message(STATUS "Using prebuilt dependencies with PREBUILT_DEPS_TAG=${PREBUILT_DEPS_TAG}")

        set(PREBUILT_DEPS_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
        if (PREBUILT_DEPS_BUILD_TYPE STREQUAL "RelWithDebInfo" OR PREBUILT_DEPS_BUILD_TYPE STREQUAL "MinSizeRel")
            set(PREBUILT_DEPS_BUILD_TYPE "Release")
        endif()

        set(PREBUILT_DEPS_FILENAME "${BUILD_PLATFORM}_${PREBUILT_DEPS_BUILD_TYPE}_${BUILD_ARCHITECTURE}.zip")
        set(PREBUILT_DEPS_DIR "${CMAKE_CURRENT_BINARY_DIR}/dependencies")
        if (NOT EXISTS "${PREBUILT_DEPS_DIR}/${PREBUILT_DEPS_FILENAME}")
            download_prebuilt_dependencies("${PREBUILT_DEPS_TAG}" "${PREBUILT_DEPS_FILENAME}" "${PREBUILT_DEPS_DIR}")
        endif()

        if(CMAKE_FIND_ROOT_PATH)
            list(APPEND CMAKE_FIND_ROOT_PATH "${PREBUILT_DEPS_DIR}")
            list(APPEND CMAKE_MODULE_PATH "/")
            list(APPEND CMAKE_PREFIX_PATH "/")
        else()
            list(APPEND CMAKE_MODULE_PATH "${PREBUILT_DEPS_DIR}")
            list(APPEND CMAKE_PREFIX_PATH "${PREBUILT_DEPS_DIR}")
        endif()

        set(ZLIB_USE_STATIC_LIBS ON)
        find_package(ZLIB REQUIRED)

        if (NOT BUILD_PLATFORM STREQUAL "android") # TODO(captainurist) : add more android prebuilt libs
            find_package(SDL2 CONFIG REQUIRED GLOBAL)
            add_library(SDL2OE INTERFACE)
            target_link_libraries(SDL2OE INTERFACE SDL2::SDL2)
            if(TARGET SDL2::SDL2main) # Not all platforms have SDL2main.
                target_link_libraries(SDL2OE INTERFACE SDL2::SDL2main)
            endif()
            add_library(SDL2::SDL2OE ALIAS SDL2OE)

            find_package(OpenAL CONFIG REQUIRED GLOBAL)
        endif()

        find_package(FFmpeg REQUIRED)
    elseif(OE_USE_DUMMY_DEPENDENCIES)
        # Just create dummy libs so that configure pass works. We won't be building anything.
        add_library(ZLIB INTERFACE)
        add_library(ZLIB::ZLIB ALIAS ZLIB)
        add_library(OpenAL INTERFACE)
        add_library(OpenAL::OpenAL ALIAS OpenAL)
        add_library(SDL2OE INTERFACE)
        add_library(SDL2::SDL2OE ALIAS SDL2OE)
        set(SDL2_FOUND ON)
        set(OPENAL_FOUND ON)
    else()
        message(STATUS "Not using prebuilt dependencies")
        find_package(FFmpeg REQUIRED)
        find_package(ZLIB REQUIRED)
    endif()

    # On Android we somehow get OpenGL available by default, despite it not being findable by find_package. So we
    # just create a dummy lib.
    if(OE_USE_DUMMY_DEPENDENCIES OR BUILD_PLATFORM STREQUAL "android")
        add_library(OpenGL_GL INTERFACE)
        add_library(OpenGL::GL ALIAS OpenGL_GL)
    else()
        find_package(OpenGL REQUIRED)
    endif()
endmacro()
