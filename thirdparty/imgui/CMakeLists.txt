cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
# Create the imgui libraries here since the submodule doesn't contain a CMakeLists.txt

set(IMGUI_SOURCES
  imgui/imgui_draw.cpp
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_opengl3.cpp
  imgui/backends/imgui_impl_sdl2.cpp
  )

set(IMGUI_HEADERS
  imgui/imconfig.h
  imgui/imgui.h
  imgui/imgui_internal.h
  imgui/imstb_rectpack.h
  imgui/imstb_textedit.h
  imgui/imstb_truetype.h
  imgui/backends/imgui_impl_opengl3.h
  imgui/backends/imgui_impl_sdl2.h
  )
add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})
target_link_libraries(imgui PRIVATE SDL2::SDL2OE)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
