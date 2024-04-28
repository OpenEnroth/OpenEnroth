#include "RenderStatsDebugView.h"

#include <Engine/Graphics/Renderer/Renderer.h>
#include <Engine/Graphics/Nuklear.h>

#include <Engine/Party.h>

#include <nuklear_config.h> // NOLINT: not a C system header.

RenderStatsDebugView::RenderStatsDebugView(Renderer& renderer) : _renderer(renderer) {
}

void RenderStatsDebugView::update(Nuklear &nuklear) {
    static int i = 0;
    ++i;
    nk_context *context = nuklear.getContext();

    nk_begin(context, "Test", nk_rect(100, 100, 100, 100), NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_SCALABLE);
    auto partyPosition = fmt::format("Party position:         {:.2f} {:.2f} {:.2f}", pParty->pos.x, pParty->pos.y, pParty->pos.z);
    nk_layout_row_dynamic(context, 0, 1);
    nk_label_colored(context, partyPosition.c_str(), NK_TEXT_ALIGN_CENTERED, { 255, 255, 255, 255 });

    nk_end(context);

    /*
    if (render_framerate) {
        pPrimaryWindow->DrawText(assets->pFontArrus.get(), { 494, 0 }, colorTable.White, fmt::format("FPS: {: .4f}", framerate));
    }

    pPrimaryWindow->DrawText(assets->pFontArrus.get(), { 300, 0 }, colorTable.White, fmt::format("DrawCalls: {}", render->drawcalls));
    render->drawcalls = 0;


    int debug_info_offset = 16;
    pPrimaryWindow->DrawText(assets->pFontArrus.get(), { 16, debug_info_offset }, colorTable.White,
        fmt::format("Party position:         {:.2f} {:.2f} {:.2f}", pParty->pos.x, pParty->pos.y, pParty->pos.z));
    debug_info_offset += 16;

    pPrimaryWindow->DrawText(assets->pFontArrus.get(), { 16, debug_info_offset }, colorTable.White,
        fmt::format("Party yaw/pitch:     {} {}", pParty->_viewYaw, pParty->_viewPitch));
    debug_info_offset += 16;

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        int sector_id = pBLVRenderParams->uPartySectorID;
        pPrimaryWindow->DrawText(assets->pFontArrus.get(), { 16, debug_info_offset }, colorTable.White,
            fmt::format("Party Sector ID:       {}/{}\n", sector_id, pIndoor->pSectors.size()));
        debug_info_offset += 16;
    }

    std::string floor_level_str;

    if (uGameState == GAME_STATE_CHANGE_LOCATION) {
        floor_level_str = "Loading Level!";
    } else if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        int uFaceID;
        int sector_id = pBLVRenderParams->uPartySectorID;
        int floor_level = BLV_GetFloorLevel(pParty->pos.toInt()/* + Vec3i(0,0,40) *//*, sector_id, &uFaceID);
        floor_level_str = fmt::format("BLV_GetFloorLevel: {}   face_id {}\n", floor_level, uFaceID);
    } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        bool on_water = false;
        int bmodel_pid;
        int floor_level = ODM_GetFloorLevel(pParty->pos.toInt(), 0, &on_water, &bmodel_pid, false);
        floor_level_str = fmt::format(
            "ODM_GetFloorLevel: {}   on_water: {}  on: {}\n",
            floor_level, on_water ? "true" : "false",
            bmodel_pid == 0
            ? "---"
            : fmt::format("BModel={} Face={}", bmodel_pid >> 6, bmodel_pid & 0x3F)
        );
    }

    pPrimaryWindow->DrawText(assets->pFontArrus.get(), { 16, debug_info_offset }, colorTable.White, floor_level_str);
    */
}
