function(init_check_lua_style)
    if(OE_CHECK_LUA_STYLE)
        find_program(OE_LLS_COMMAND lua-language-server)
        set(OE_LLS_CHECK_COMMAND "${PROJECT_SOURCE_DIR}/lls-check.py" CACHE FILEPATH "Lua Language Server check script")
        if(OE_LLS_COMMAND)
            message("Using lua-language-server at '${OE_LLS_COMMAND}'")
        else()
            message("Could not find lua-language-server")
        endif()
    endif()
endfunction()

function(source_check_lua_style TARGET)
    if(OE_CHECK_LUA_STYLE)
        set(TARGET_NAME "check_style_lua_${TARGET}")
        
        set(LOG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lls-check/${TARGET}")
        add_custom_target(${TARGET_NAME}
                COMMAND Python::Interpreter ${OE_LLS_CHECK_COMMAND} ${OE_LLS_COMMAND} ${PROJECT_SOURCE_DIR} ${LOG_OUTPUT_DIR} "${PROJECT_SOURCE_DIR}/.luarc.json"
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

        add_dependencies(check_style ${TARGET_NAME})
        set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "check_style")
    endif()
endfunction()

function(target_check_lua_style TARGET)
    if(OE_CHECK_LUA_STYLE AND OE_LLS_COMMAND)
        source_check_lua_style(${TARGET})
    endif()
endfunction()
