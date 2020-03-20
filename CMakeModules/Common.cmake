function(DEBUG_PRINT msg)
    if (NOT (CMAKE_BUILD_TYPE MATCHES "RELEASE"))
        message(STATUS ${msg})
    endif()
endfunction()

function(ADD_GLOBAL_DEPENDENCY filename)
    if (NOT EXISTS ${filename})
        message(STATUS "ADD_GLOBAL_DEPENDENCY(${filename}): file does not exist")
    else()
        list(APPEND GLOBAL_DEPENDENCIES "${filename}")
        set (GLOBAL_DEPENDENCIES ${GLOBAL_DEPENDENCIES} PARENT_SCOPE)
    endif()
endfunction()

function(RESOLVE_DEPENDENCIES targetName)
    foreach(dep ${GLOBAL_DEPENDENCIES})
        message(STATUS "Copying binary dependency ${dep} to $<TARGET_FILE_DIR:${targetName}>")
        add_custom_command(
            TARGET ${targetName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${dep}" $<TARGET_FILE_DIR:${targetName}>
        )
    endforeach()
endfunction()
