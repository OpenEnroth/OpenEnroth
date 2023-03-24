cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(INCLUDE_THIRDPARTY_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/luajit/luajit/src
        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include
        ${CMAKE_CURRENT_SOURCE_DIR}/mini/src
        PARENT_SCOPE)

set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)

add_subdirectory(fast_float)
add_subdirectory(fmt)
add_subdirectory(glad)
add_subdirectory(glm)
add_subdirectory(luajit)
add_subdirectory(magic_enum)
add_subdirectory(nuklear)
add_subdirectory(cli11)
add_subdirectory(nlohmann_json)
add_subdirectory(mio)
if(ENABLE_TESTS)
    add_subdirectory(googletest)
endif()

set_target_properties(libluajit PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}" LINK_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
