cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

if(WIN32)
    add_custom_target(OpenEnroth_Package
            ${CMAKE_COMMAND} -E tar cf ${CMAKE_CURRENT_BINARY_DIR}/OpenEnroth.zip --format=zip $<TARGET_FILE_NAME:OpenEnroth>
                $<$<CONFIG:Debug,RelWithDebInfo>:$<TARGET_PDB_FILE_NAME:OpenEnroth>> # Only these configurations link with /debug.
            DEPENDS OpenEnroth
            WORKING_DIRECTORY $<TARGET_FILE_DIR:OpenEnroth>
            VERBATIM
            COMMAND_EXPAND_LISTS)
elseif(APPLE)
    add_custom_target(OpenEnroth_Package
            ${CMAKE_CURRENT_SOURCE_DIR}/macos/make_dmg.sh $<TARGET_BUNDLE_DIR:OpenEnroth> $<TARGET_BUNDLE_CONTENT_DIR:OpenEnroth>/Resources/OpenEnroth.icns
                ${CMAKE_CURRENT_SOURCE_DIR}/macos/background.jpg ${CMAKE_CURRENT_BINARY_DIR}/OpenEnroth.dmg
            DEPENDS OpenEnroth
            VERBATIM)
endif()
