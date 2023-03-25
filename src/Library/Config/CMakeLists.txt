cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_CONFIG_SOURCES
        Config.cpp
        ConfigSection.cpp)

set(LIBRARY_CONFIG_HEADERS
        AbstractConfigValue.h
        Config.h
        ConfigFwd.h
        ConfigSection.h
        ConfigValue.h)

add_library(library_config STATIC ${LIBRARY_CONFIG_SOURCES} ${LIBRARY_CONFIG_HEADERS})
target_link_libraries(library_config utility library_serialization)
target_check_style(library_config)
