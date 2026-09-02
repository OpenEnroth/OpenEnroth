cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_RESOURCES_SOURCES
        EngineFileSystem.cpp
        LOD.cpp
        LodSpriteCache.cpp
        LodTextureCache.cpp
        ResourceManager.cpp)

set(ENGINE_RESOURCES_HEADERS
        EngineFileSystem.h
        LOD.h
        LodSpriteCache.h
        LodTextureCache.h
        ResourceManager.h)

add_library(engine_resources STATIC ${ENGINE_RESOURCES_SOURCES} ${ENGINE_RESOURCES_HEADERS})
target_link_libraries(engine_resources PUBLIC library_lod library_lod_formats library_filesystem_interface utility)
target_check_style(engine_resources)
