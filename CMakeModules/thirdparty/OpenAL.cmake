set(OAL_DIR "${LIBRARY_DIR}/openal-1.1")
set(OAL_BIN_DIR "${OAL_DIR}/bin")
set(OAL_LIB_DIR "${OAL_DIR}/lib")
set(OAL_INCLUDE_DIR "${OAL_DIR}/include")

# hack to be able to create a convenience library without sources
file(WRITE ${OAL_DIR}/openal.c "")
add_library(
    openal
    ${OAL_DIR}/openal.c
)

set_target_properties(
    openal PROPERTIES
    PUBLIC_HEADER "${OAL_INCLUDE_DIR}/AL/al.h"
)

target_link_libraries(
    openal
    "${OAL_LIB_DIR}/OpenAL32.lib"
)

include_directories(
    "${OAL_INCLUDE_DIR}"
)

# OpenAL interface
ADD_GLOBAL_DEPENDENCY("${OAL_BIN_DIR}/OpenAL32.dll")
# software sound driver
ADD_GLOBAL_DEPENDENCY("${OAL_BIN_DIR}/soft_oal.dll")
