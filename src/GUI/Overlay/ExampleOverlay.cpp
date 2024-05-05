#include "ExampleOverlay.h"

#include <nuklear_config.h> // NOLINT: not a C system header.

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Party.h"

void ExampleOverlay::update(nk_context *ctx) {
    nk_begin(ctx, "Party Position", nk_rect(100, 100, 320, 80), NK_WINDOW_SCALABLE | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE);
    auto partyPosition = fmt::format("PARTY POSITION: {:.2f} {:.2f} {:.2f}", pParty->pos.x, pParty->pos.y, pParty->pos.z);
    nk_layout_row_dynamic(ctx, 0, 1);
    nk_label_colored(ctx, partyPosition.c_str(), NK_TEXT_ALIGN_CENTERED, { 255, 255, 255, 255 });
    nk_end(ctx);
}
