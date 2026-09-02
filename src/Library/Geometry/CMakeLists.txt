cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_GEOMETRY_SOURCES)

set(LIBRARY_GEOMETRY_HEADERS
        BBox.h
        Margins.h
        Plane.h
        Point.h
        Rect.h
        Size.h
        Vec.h)

add_library(library_geometry INTERFACE ${LIBRARY_GEOMETRY_SOURCES} ${LIBRARY_GEOMETRY_HEADERS})
target_link_libraries(library_geometry INTERFACE utility)
target_check_style(library_geometry)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_GEOMETRY_SOURCES Tests/Rect_ut.cpp)

    add_library(test_library_geometry OBJECT ${TEST_LIBRARY_GEOMETRY_SOURCES})
    target_link_libraries(test_library_geometry PUBLIC testing_unit library_geometry)

    target_check_style(test_library_geometry)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_geometry)
endif()
