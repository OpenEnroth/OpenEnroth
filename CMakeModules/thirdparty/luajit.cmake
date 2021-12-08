# hack to be able to create a convenience library without sources
file(WRITE ${LIB_DIR}/luajit.c "")
add_library(
    luajit STATIC
    ${LIB_DIR}/luajit.c
)

if (WIN32)
set(LUAJIT_DIR "${LIBRARY_DIR}/luajit-2.1.0")
set(LUAJIT_BIN_DIR "${LUAJIT_DIR}/bin")
set(LUAJIT_LIB_DIR "${LUAJIT_DIR}/lib")
set(LUAJIT_INCLUDE_DIR "${LUAJIT_DIR}/include")

target_link_libraries(
    luajit
    "${LUAJIT_LIB_DIR}/luajit.lib"
    "${LUAJIT_LIB_DIR}/lua51.lib"
)

include_directories(
    "${LUAJIT_INCLUDE_DIR}"
)

ADD_GLOBAL_DEPENDENCY("${LUAJIT_BIN_DIR}/luajit.dll")
ADD_GLOBAL_DEPENDENCY("${LUAJIT_BIN_DIR}/lua51.dll")

else()  # WIN32
find_package(luajit REQUIRED)
target_link_libraries(luajit "${LUAJIT_LIBRARIES}")
include_directories("${LUAJIT_INCLUDE_DIR}")
endif()
