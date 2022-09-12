cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(GRAPHICS_SOURCES BSPModel.cpp
                     Collisions.cpp
                     DecalBuilder.cpp
                     DecorationList.cpp
                     Image.cpp
                     ImageLoader.cpp
                     Indoor.cpp
                     Camera.cpp
                     LightmapBuilder.cpp
                     LightsStack.cpp
                     Outdoor.cpp
                     Overlays.cpp
                     Nuklear.cpp
                     PaletteManager.cpp
                     ParticleEngine.cpp
                     PCX.cpp
                     RenderBase.cpp
                     Sprites.cpp
                     PortalFunctions.cpp
                     ClippingFunctions.cpp
                     Viewport.cpp
                     Vis.cpp
                     Weather.cpp
                     Level/Decoration.cpp
                     OpenGL/RenderOpenGL.cpp
                     OpenGL/TextureOpenGL.cpp
                     IRenderFactory.cpp
                     HWLContainer.cpp)

set(GRAPHICS_HEADERS BSPModel.h
                     Collisions.h
                     DecalBuilder.h
                     DecorationList.h
                     Image.h
                     ImageFormatConverter.h
                     ImageLoader.h
                     Indoor.h
                     Camera.h
                     IRender.h
                     LightmapBuilder.h
                     LightsStack.h
                     Outdoor.h
                     Overlays.h
                     Nuklear.h
                     PaletteManager.h
                     ParticleEngine.h
                     PCX.h
                     Polygon.h
                     RenderBase.h
                     Sprites.h
                     PortalFunctions.h
                     ClippingFunctions.h
                     Texture.h
                     Viewport.h
                     Vis.h
                     Weather.h
                     Level/Decoration.h
                     OpenGL/RenderOpenGL.h
                     OpenGL/TextureOpenGL.h
                     OpenGL/GLShaderLoader.h
                     IRenderFactory.h
                     HWLContainer.h)

if(WIN32)
  list(APPEND GRAPHICS_SOURCES Direct3D/Render.cpp
                               Direct3D/RenderD3D.cpp
                               Direct3D/TextureD3D.cpp)

  list(APPEND GRAPHICS_HEADERS Direct3D/Render.h
                               Direct3D/RenderD3D.h
                               Direct3D/TextureD3D.h)
endif()

add_library(graphics STATIC ${GRAPHICS_SOURCES} ${GRAPHICS_HEADERS})
target_check_style(graphics)

find_package(OpenGL REQUIRED)

target_link_libraries(graphics ${OPENGL_opengl_LIBRARY} ${OPENGL_glu_LIBRARY})

if(WIN32)
  target_link_libraries(graphics ddraw.lib dxguid.lib)
endif()

message(VERBOSE "OPENGL_opengl_LIBRARY: ${OPENGL_opengl_LIBRARY}")
message(VERBOSE "OPENGL_glu_LIBRARY: ${OPENGL_glu_LIBRARY}")
