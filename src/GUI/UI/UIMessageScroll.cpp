#include "UIMessageScroll.h"

#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Tables/MessageScrollTable.h"
#include "Engine/Tables/ItemTable.h"

#include "Engine/Graphics/Renderer/Renderer.h"

#include "GUI/GUIFont.h"

void CreateMsgScrollWindow(ItemId mscroll_id) {
    if (!pGUIWindow_ScrollWindow && isMessageScroll(mscroll_id)) {
        pGUIWindow_ScrollWindow = new GUIWindow_MessageScroll({0, 0}, render->GetRenderDimensions(), mscroll_id, "");
    }
}

void GUIWindow_MessageScroll::Update() {
    GUIWindow_MessageScroll &a1 = *pGUIWindow_ScrollWindow;
    a1.sHint.clear();
    a1.uFrameX = 1;
    a1.uFrameY = 1;
    a1.uFrameWidth = 468;
    int textHeight = assets->pFontSmallnum->CalcTextHeight(pMessageScrolls[pGUIWindow_ScrollWindow->scroll_type], a1.uFrameWidth, 0);
    unsigned int v0 = textHeight + 2 * (unsigned char)assets->pFontCreate->GetHeight() + 24;
    a1.uFrameHeight = v0;
    if ((v0 + a1.uFrameY) > 479) {
        v0 = 479 - a1.uFrameY;
        a1.uFrameHeight = 479 - a1.uFrameY;
    }
    a1.uFrameZ = a1.uFrameWidth + a1.uFrameX - 1;
    a1.uFrameW = v0 + a1.uFrameY - 1;
    a1.DrawMessageBox(0);
    a1.uFrameX += 12;
    a1.uFrameWidth -= 28;
    a1.uFrameY += 12;
    a1.uFrameHeight -= 12;
    a1.uFrameZ = a1.uFrameWidth + a1.uFrameX - 1;
    a1.uFrameW = a1.uFrameHeight + a1.uFrameY - 1;

    const std::string &name = pItemTable->items[pGUIWindow_ScrollWindow->scroll_type].name;

    a1.DrawTitleText(assets->pFontCreate.get(), 0, 0, colorTable.White, fmt::format("{::}{}\f00000\n", colorTable.PaleCanary.tag(), name), 3);
    a1.DrawText(assets->pFontSmallnum.get(), {1, assets->pFontCreate->GetHeight() - 3}, colorTable.White, pMessageScrolls[pGUIWindow_ScrollWindow->scroll_type]);
}
