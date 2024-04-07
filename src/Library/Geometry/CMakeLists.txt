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
