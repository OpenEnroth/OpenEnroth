cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(GRAPHICS_SOURCES BSPModel.cpp
                     DecalBuilder.cpp
                     DecorationList.cpp
                     Image.cpp
                     ImageLoader.cpp
                     Indoor.cpp
                     IndoorCameraD3D.cpp
                     LightmapBuilder.cpp
                     LightsStack.cpp
                     Outdoor.cpp
                     Overlays.cpp
                     PaletteManager.cpp
                     ParticleEngine.cpp
                     PCX.cpp
                     Direct3D/Render.cpp
                     RenderBase.cpp
                     Direct3D/RenderD3D.cpp
                     Sprites.cpp
                     stru10.cpp
                     stru9.cpp
                     Viewport.cpp
                     Vis.cpp
                     Weather.cpp
                     Direct3D/TextureD3D.cpp
                     Level/Decoration.cpp
                     OpenGL/RenderOpenGL.cpp
                     OpenGL/TextureOpenGL.cpp
                     IRenderConfig.cpp
                     IRenderConfigFactory.cpp
                     IRenderFactory.cpp
                     HWLContainer.cpp)

set(GRAPHICS_HEADERS BSPModel.h
                     DecalBuilder.h
                     DecorationList.h
                     Image.h
                     ImageFormatConverter.h
                     ImageLoader.h
                     Indoor.h
                     IndoorCameraD3D.h
                     IRender.h
                     LightmapBuilder.h
                     Lights.h
                     Outdoor.h
                     Overlays.h
                     PaletteManager.h
                     ParticleEngine.h
                     PCX.h
                     Polygon.h
                     Direct3D/Render.h
                     RenderBase.h
                     Direct3D/RenderD3D.h
                     Sprites.h
                     stru10.h
                     stru9.h
                     Texture.h
                     Viewport.h
                     Vis.h
                     Weather.h
                     Direct3D/TextureD3D.h
                     Level/Decoration.h
                     OpenGL/RenderOpenGL.h
                     OpenGL/TextureOpenGL.h
                     IRenderConfig.h
                     IRenderConfigFactory.h
                     IRenderFactory.h
                     HWLContainer.h)

if(WIN32)
else()
endif()

add_library(graphics STATIC ${GRAPHICS_SOURCES} ${GRAPHICS_HEADERS})
add_dependencies(graphics sdl2)
target_check_style(graphics)