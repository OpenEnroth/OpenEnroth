cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(CONFIG_SOURCES Config.cpp)

set(CONFIG_HEADERS Config.h
                   ConfigFwd.h
                   ConfigSection.h
                   ConfigSerialization.h
                   ConfigValue.h)

add_library(config STATIC ${CONFIG_SOURCES} ${CONFIG_HEADERS})
target_check_style(config)
