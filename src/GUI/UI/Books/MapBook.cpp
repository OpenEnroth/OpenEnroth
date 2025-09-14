#include "MapBook.h"

#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/MapInfo.h"
#include "Engine/Engine.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIGame.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/Math/TrigLut.h"

void DrawBook_Map_sub(int tl_x, int tl_y, int br_x, int br_y);

GraphicsImage *ui_book_map_background = nullptr;

GUIWindow_MapBook::GUIWindow_MapBook() {
    this->eWindowType = WindowType::WINDOW_MapsBook;
    viewparams->sViewCenterX = pParty->pos.x;
    viewparams->sViewCenterY = pParty->pos.y;
    pChildBooksOverlay = new GUIWindow_BooksButtonOverlay({546, 353}, {0, 0}, pBtn_Maps);

    ui_book_map_background = assets->getImage_ColorKey("sbmap");
    ui_book_button1_on = assets->getImage_Alpha("zoom-on");
    ui_book_button2_on = assets->getImage_Alpha("zoot-on");
    ui_book_button3_on = assets->getImage_Alpha("tabNon");
    ui_book_button4_on = assets->getImage_Alpha("tabSon");
    ui_book_button5_on = assets->getImage_Alpha("tabEon");
    ui_book_button6_on = assets->getImage_Alpha("tabWon");
    ui_book_button1_off = assets->getImage_Alpha("zoom-off");
    ui_book_button2_off = assets->getImage_Alpha("zoot-off");
    ui_book_button3_off = assets->getImage_Alpha("tabNoff");
    ui_book_button4_off = assets->getImage_Alpha("tabSoff");
    ui_book_button5_off = assets->getImage_Alpha("tabEoff");
    ui_book_button6_off = assets->getImage_Alpha("tabWoff");

    pBtn_Book_1 = CreateButton({pViewport->viewportTL_X + 398, pViewport->viewportTL_Y + 1}, {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_ZOOM_IN), Io::InputAction::ZoomIn, localization->GetString(LSTR_ZOOM_IN), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton({pViewport->viewportTL_X + 398, pViewport->viewportTL_Y + 38}, {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_ZOOM_OUT), Io::InputAction::ZoomOut, localization->GetString(LSTR_ZOOM_OUT), {ui_book_button2_on});
    pBtn_Book_3 = CreateButton({pViewport->viewportTL_X + 397, pViewport->viewportTL_Y + 113}, {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_SCROLL_UP), Io::InputAction::DialogUp, localization->GetString(LSTR_SCROLL_UP), {ui_book_button3_on});
    pBtn_Book_4 = CreateButton({pViewport->viewportTL_X + 397, pViewport->viewportTL_Y + 150}, {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_SCROLL_DOWN), Io::InputAction::DialogDown, localization->GetString(LSTR_SCROLL_DOWN), {ui_book_button4_on});
    pBtn_Book_5 = CreateButton({pViewport->viewportTL_X + 397, pViewport->viewportTL_Y + 188}, {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_SCROLL_RIGHT), Io::InputAction::DialogRight, localization->GetString(LSTR_SCROLL_RIGHT), {ui_book_button5_on});
    pBtn_Book_6 = CreateButton({pViewport->viewportTL_X + 397, pViewport->viewportTL_Y + 226}, {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_SCROLL_LEFT), Io::InputAction::DialogLeft, localization->GetString(LSTR_SCROLL_LEFT), {ui_book_button6_on});
}

void GUIWindow_MapBook::Update() {
    render->DrawTextureNew(471 /  640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
    render->DrawTextureNew(pViewport->viewportTL_X / 640.0f, pViewport->viewportTL_Y / 480.0f, ui_book_map_background);

    if ((_bookButtonClicked && _bookButtonAction == BOOK_ZOOM_IN) || viewparams->uMapBookMapZoom / 128 >= 12) {
        render->DrawTextureNew((pViewport->viewportTL_X + 408) / 640.0f, (pViewport->viewportTL_Y + 2) / 480.0f, ui_book_button1_off);
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 398) / 640.0f, (pViewport->viewportTL_Y + 1) / 480.0f, ui_book_button1_on);
    }

    if ((_bookButtonClicked && _bookButtonAction == BOOK_ZOOM_OUT) || viewparams->uMapBookMapZoom / 128 <= 3) {
        render->DrawTextureNew((pViewport->viewportTL_X + 408) / 640.0f, (pViewport->viewportTL_Y + 38) / 480.0f, ui_book_button2_off);
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 398) / 640.0f, (pViewport->viewportTL_Y + 38) / 480.0f, ui_book_button2_on);
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_SCROLL_UP) {
        render->DrawTextureNew((pViewport->viewportTL_X + 408) / 640.0f, (pViewport->viewportTL_Y + 113) / 480.0f, ui_book_button3_off);
        viewparams->MapViewUp();
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 398) / 640.0f, (pViewport->viewportTL_Y + 113) / 480.0f, ui_book_button3_on);
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_SCROLL_DOWN) { // Button 4
        render->DrawTextureNew((pViewport->viewportTL_X + 408) / 640.0f, (pViewport->viewportTL_Y + 150) / 480.0f, ui_book_button4_off);
        viewparams->MapViewDown();
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 399) / 640.0f, (pViewport->viewportTL_Y + 150) / 480.0f, ui_book_button4_on);
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_SCROLL_RIGHT) {
        render->DrawTextureNew((pViewport->viewportTL_X + 408) / 640.0f, (pViewport->viewportTL_Y + 188) / 480.0f, ui_book_button5_off);
        viewparams->MapViewRight();
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 397) / 640.0f, (pViewport->viewportTL_Y + 188) / 480.0f, ui_book_button5_on);
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_SCROLL_LEFT) {
        render->DrawTextureNew((pViewport->viewportTL_X + 408) / 640.0f, (pViewport->viewportTL_Y + 226) / 480.0f, ui_book_button6_off);
        viewparams->MapViewLeft();
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 397) / 640.0f, (pViewport->viewportTL_Y + 226) / 480.0f, ui_book_button6_on);
    }

    if (_bookButtonClicked) {
        if (_bookButtonAction == BOOK_ZOOM_IN) {
            viewparams->CenterOnPartyZoomIn();
        } else if (_bookButtonAction == BOOK_ZOOM_OUT) {
            viewparams->CenterOnPartyZoomOut();
        }
    }

    if (_bookButtonClicked) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }

    _bookButtonClicked = false;

    render->DrawTextureNew(75 / 640.0f, 22 / 480.0f, ui_book_map_frame);
    DrawBook_Map_sub(97, 49, 361, 313);
    render->ResetUIClipRect();

    GUIWindow map_window;
    map_window.uFrameWidth = pViewport->viewportWidth;
    map_window.uFrameHeight = pViewport->viewportHeight;
    map_window.uFrameX = pViewport->viewportTL_X;
    map_window.uFrameY = pViewport->viewportTL_Y;
    map_window.uFrameZ = pViewport->viewportBR_X;
    map_window.uFrameW = pViewport->viewportBR_Y;

    if (engine->_currentLoadedMapId != MAP_INVALID) {
        map_window.DrawTitleText(assets->pFontBookTitle.get(), -14, 12, ui_book_map_title_color, pMapStats->pInfos[engine->_currentLoadedMapId].name, 3);
    }

    auto party_coordinates = localization->FormatString(LSTR_X_D_Y_D, static_cast<int>(pParty->pos.x), static_cast<int>(pParty->pos.y));

    map_window.uFrameX = 0;
    map_window.DrawTitleText(assets->pFontComic.get(), 0, 320, ui_book_map_coordinates_color, party_coordinates, 0);
}

//----- (00442955) --------------------------------------------------------
void DrawBook_Map_sub(int tl_x, int tl_y, int br_x, int br_y) {
    // TODO(captainurist): this needs to be merged with GameUI_DrawMinimap

    Vec2i screenCenter((tl_x + br_x) / 2, (tl_y + br_y) / 2);

    render->SetUIClipRect(Recti(tl_x, tl_y, br_x - tl_x, br_y - tl_y));

    Vec2f center(viewparams->sViewCenterX, viewparams->sViewCenterY);

    if (viewparams->uMapBookMapZoom == 384)
        center = Vec2f(viewparams->indoor_center_x, viewparams->indoor_center_y);

    if (uCurrentlyLoadedLevelType != LEVEL_INDOOR) {  // outdoors
        int screenWidth = br_x - tl_x + 1;
        int screenHeight = br_y - tl_y + 1;

        int loc_power = ImageHelper::GetWidthLn2(viewparams->location_minimap);
        int scale_increment = (1 << (loc_power + 16)) / viewparams->uMapBookMapZoom;
        double MapSizeScale = (double)(1 << (16 - loc_power));
        int stepX_r_resets =
            (unsigned int)(int64_t)
            ((double)(center.x - maxPartyAxisDistance / (viewparams->uMapBookMapZoom / 384) + 32768) / MapSizeScale) << 16;
        int stepY_r = (int)(int64_t)
            ((double)(-center.y - maxPartyAxisDistance / (viewparams->uMapBookMapZoom / 384) + 32768) / MapSizeScale) << 16;
        int scaled_posY = stepY_r >> 16;

        static GraphicsImage *minimaptemp = nullptr;
        if (!minimaptemp) {
            minimaptemp = GraphicsImage::Create(screenWidth, screenHeight);
        }
        Color *minitempix = minimaptemp->rgba().pixels().data();
        const Color *minimap_pixels = viewparams->location_minimap->rgba().pixels().data();
        int textr_width = viewparams->location_minimap->width();

        // nearest neiborhood scaling
        // if (texture8_data)
        // TODO(pskelton): stretch texture dont scale it
        {
            for (unsigned i = 0; i < screenHeight; ++i) {
                int map_tile_Y = (scaled_posY - 80) / 4;
                int stepX_r = stepX_r_resets;
                for (unsigned j = 0; j < screenWidth; ++j) {
                    int scaled_posX = stepX_r >> 16;
                    int map_tile_X = (scaled_posX - 80) / 4;
                    if (!pOutdoor->IsMapCellFullyRevealed(map_tile_X, map_tile_Y)) {
                        if (pOutdoor->IsMapCellPartiallyRevealed(map_tile_X,
                            map_tile_Y)) {
                            if (!((i + screenCenter.x + j) % 2))
                                minitempix[j + i * screenWidth] = colorTable.GrayBlack;
                            else
                                minitempix[j + i * screenWidth] = minimap_pixels[scaled_posX + scaled_posY * textr_width];
                        } else {
                            minitempix[j + i * screenWidth] = colorTable.Black;
                        }
                    } else {
                        minitempix[j + i * screenWidth] = minimap_pixels[scaled_posX + scaled_posY * textr_width];
                    }
                    stepX_r += scale_increment;
                }
                stepY_r += scale_increment;
                scaled_posY = stepY_r >> 16;
            }
        }

        render->Update_Texture(minimaptemp);
        render->DrawTextureNew(tl_x / 640., tl_y / 480., minimaptemp);
    } else {  // indoors
        if (!pIndoor->pMapOutlines.empty()) {
            render->BeginLines2D();
            for (unsigned i = 0; i < pIndoor->pMapOutlines.size(); ++i) {
                BLVMapOutline *pOutline = &pIndoor->pMapOutlines[i];

                if (pIndoor->pFaces[pOutline->uFace1ID].Visible() &&
                    pIndoor->pFaces[pOutline->uFace2ID].Visible()) {
                    if (pIndoor->pFaces[pOutline->uFace1ID].uAttributes & FACE_SeenByParty ||
                        pIndoor->pFaces[pOutline->uFace2ID].uAttributes & FACE_SeenByParty) {
                        pOutline->uFlags = pOutline->uFlags | 1;
                        pIndoor->_visible_outlines[i >> 3] |= 1 << (7 - i % 8);

                        Vec2f Vert1 = (pIndoor->pVertices[pOutline->uVertex1ID].xy() - center) / 65536.0f;
                        Vec2f Vert2 = (pIndoor->pVertices[pOutline->uVertex2ID].xy() - center) / 65536.0f;

                        Vert1.y = -Vert1.y;
                        Vert2.y = -Vert2.y;

                        Vec2i linea = screenCenter + (Vert1 * viewparams->uMapBookMapZoom).toInt();
                        Vec2i lineb = screenCenter + (Vert2 * viewparams->uMapBookMapZoom).toInt();

                        render->RasterLine2D(linea, lineb, colorTable.Black);
                    }
                }
            }
            render->EndLines2D();
        }
    }

    // Direction arrow drawing
    int ArrowXPos = (pParty->pos.x - center.x) * viewparams->uMapBookMapZoom / 65536.0f + screenCenter.x - 3;
    int ArrowYPos = screenCenter.y - ((pParty->pos.y - center.y) * viewparams->uMapBookMapZoom / 65536.0f) - 3;
    bool DrawArrow = 1;

    if (ArrowXPos >= (signed int)tl_x) {
        if (ArrowXPos > (signed int)br_x) {
            if ((signed int)(ArrowXPos - 3) > (signed int)br_x) DrawArrow = 0;
            ArrowXPos = br_x;
        }
    } else {
        if ((signed int)(ArrowXPos + 3) < (signed int)tl_x) DrawArrow = 0;
        ArrowXPos = tl_x;
    }

    if (ArrowYPos >= (signed int)tl_y) {
        if (ArrowYPos > br_y) {
            if ((ArrowYPos - 3) > br_y) DrawArrow = 0;
            ArrowYPos = br_y;
        }
    } else {
        if ((ArrowYPos + 3) < (signed int)tl_y) DrawArrow = 0;
        ArrowYPos = tl_y;
    }

    if (DrawArrow == 1) {
        int ArrowOctant = 0;
        int PartyDirection = pParty->_viewYaw & TrigLUT.uDoublePiMask;
        if (PartyDirection <= 1920) ArrowOctant = 6;
        if (PartyDirection < 1664) ArrowOctant = 5;
        if (PartyDirection <= 1408) ArrowOctant = 4;
        if (PartyDirection < 1152) ArrowOctant = 3;
        if (PartyDirection <= 896) ArrowOctant = 2;
        if (PartyDirection < 640) ArrowOctant = 1;
        if (PartyDirection <= 384) ArrowOctant = 0;
        if (PartyDirection < 128 || PartyDirection > 1920) ArrowOctant = 7;

        render->DrawTransparentRedShade(ArrowXPos / 640.0f, ArrowYPos / 480.0f, game_ui_minimap_dirs[ArrowOctant]);
    }

    if (!pLevelDecorations.empty()) {
        render->BeginLines2D();
        for (unsigned i = 0; i < (signed int)pLevelDecorations.size(); ++i) {
            if (pLevelDecorations[i].uFlags & LEVEL_DECORATION_VISIBLE_ON_MAP) {
                Vec2f decPos = (pLevelDecorations[i].vPosition.xy() - center) / 65536.0f;
                decPos.y = -decPos.y;

                Vec2i screenPos = screenCenter + (decPos * viewparams->uMapBookMapZoom).toInt();

                if (viewparams->uMapBookMapZoom > 512) {
                    render->RasterLine2D(screenPos + Pointi(-1, -1), screenPos + Pointi(-1, 1), colorTable.White);
                    render->RasterLine2D(screenPos + Pointi(0, -1), screenPos + Pointi(0, 1), colorTable.White);
                    render->RasterLine2D(screenPos + Pointi(1, -1), screenPos + Pointi(1, 1), colorTable.White);
                } else {
                    render->RasterLine2D(screenPos, screenPos, colorTable.White);
                }
            }
        }
        render->EndLines2D();
    }
}

std::string GetMapBookHintText(int mouse_x, int mouse_y) {
    int v20 = viewparams->sViewCenterX;
    int v21 = viewparams->sViewCenterY;
    if (viewparams->uMapBookMapZoom == 384) {
        v20 = viewparams->indoor_center_x;
        v21 = viewparams->indoor_center_y;
    }

    unsigned int pX = mouse_x;
    unsigned int pY = mouse_y;

    double v0 = 1.0 / (float)((signed int)viewparams->uMapBookMapZoom * 0.000015258789);

    int global_coord_X = (int64_t)((double)(pX - 229.) * v0 + (double)v20);
    int global_coord_Y = (int64_t)((double)v21 - (double)(pY - 181.) * v0);

    std::string result;

    // In the mapbook only lady Margaret dispays for defoult zoom(В
    // книге карты только Леди Маргарита всплывает при дефолтном зуме)
    int map_tile_X = std::abs(global_coord_X + maxPartyAxisDistance) / 512;
    int map_tile_Y = std::abs(global_coord_Y - maxPartyAxisDistance) / 512;
    if (pOutdoor->IsMapCellFullyRevealed(map_tile_X, map_tile_Y) &&
        uCurrentlyLoadedLevelType == LEVEL_OUTDOOR &&
        !pOutdoor->pBModels.empty()) {
        for (BSPModel &model : pOutdoor->pBModels) {
            if (int_get_vector_length(
                    std::abs((int)model.vBoundingCenter.x - global_coord_X),
                    std::abs((int)model.vBoundingCenter.y - global_coord_Y),
                    0) < model.sBoundingRadius) {
                for (ODMFace &face : model.pFaces) {
                    if (face.sCogTriggeredID) {
                        if (!(face.uAttributes & FACE_HAS_EVENT)) {
                            std::string hintString = getEventHintString(face.sCogTriggeredID);
                            if (!hintString.empty()) {
                                result = hintString;
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}
