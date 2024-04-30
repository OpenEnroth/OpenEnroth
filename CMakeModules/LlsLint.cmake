function(init_check_lua_style)
    if(OE_CHECK_LUA_STYLE)
        unset(OE_LUALANGUAGESERVER_COMMAND CACHE)
        unset(OE_LLS_CHECK_COMMAND CACHE)
        find_program(OE_LUALANGUAGESERVER_COMMAND lua-language-server)
        set(OE_LLS_CHECK_COMMAND "${PROJECT_SOURCE_DIR}/lls-check.py" CACHE FILEPATH "Lua Language Server check script")
        if(OE_LUALANGUAGESERVER_COMMAND)
            message("Lua Language Server Found")
        else()
            message("Could not find Lua Language Server")
        endif()
    endif()
endfunction()

function(source_check_lua_style TARGET)
    if(OE_CHECK_LUA_STYLE)
        set(TARGET_NAME "check_style_lua_${TARGET}")
        
        set(LOG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lls-check/${TARGET}")
        add_custom_target(${TARGET_NAME}
                COMMAND Python::Interpreter ${OE_LLS_CHECK_COMMAND} ${OE_LUALANGUAGESERVER_COMMAND} ${PROJECT_SOURCE_DIR} ${LOG_OUTPUT_DIR} "${PROJECT_SOURCE_DIR}/.luarc.json"
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

        add_dependencies(check_style ${TARGET_NAME})
        set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "check_style")
    endif()
endfunction()

function(target_check_lua_style TARGET)
    if(OE_CHECK_LUA_STYLE AND OE_LUALANGUAGESERVER_COMMAND)
        source_check_lua_style(${TARGET})

    endif()
endfunction()
