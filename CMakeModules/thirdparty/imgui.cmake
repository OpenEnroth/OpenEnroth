set(SUBMODULES_DIR "${LIB_DIR}/submodules")
set(IMGUI_DIR "${SUBMODULES_DIR}/imgui")
add_library(
    imgui STATIC 
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
)

set_target_properties(
    zlib PROPERTIES
    PUBLIC_HEADER "${IMGUI_DIR}/imgui.h"
)

include_directories(
    "${IMGUI_DIR}"
)

