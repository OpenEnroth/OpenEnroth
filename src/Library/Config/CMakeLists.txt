cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(CONFIG_SOURCES Config.cpp
                   ConfigSection.cpp)

set(CONFIG_HEADERS AbstractConfigValue.h
                   Config.h
                   ConfigFwd.h
                   ConfigSection.h
                   ConfigValue.h)

add_library(config STATIC ${CONFIG_SOURCES} ${CONFIG_HEADERS})
target_link_libraries(config utility serialization)
target_check_style(config)
