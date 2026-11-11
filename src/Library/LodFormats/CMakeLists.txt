cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_LOD_FORMATS_SOURCES
        LodFormats.cpp)

set(LIBRARY_LOD_FORMATS_HEADERS
        LodFormats.h
        LodFormatSnapshots.h
        LodImage.h
        LodSprite.h)

add_library(library_lod_formats STATIC ${LIBRARY_LOD_FORMATS_SOURCES} ${LIBRARY_LOD_FORMATS_HEADERS})
target_link_libraries(library_lod_formats PUBLIC library_serialization library_binary library_snapshots library_compression library_font utility)
target_check_style(library_lod_formats)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_LOD_FORMATS_SOURCES
            Tests/LodFont_ut.cpp)

    add_library(test_library_lod_formats OBJECT ${TEST_LIBRARY_LOD_FORMATS_SOURCES})
    target_link_libraries(test_library_lod_formats PUBLIC testing_unit library_lod_formats)

    target_check_style(test_library_lod_formats)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_lod_formats)
endif()
