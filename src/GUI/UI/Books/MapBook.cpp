#include "MapBook.h"

#include <string>
#include <bit>
#include <utility>

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
    viewparams->ClampMapViewPosition();
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

    pBtn_Book_1 = CreateButton(pViewport.topLeft() + Pointi(398, 1), {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_ZOOM_IN), INPUT_ACTION_ZOOM_IN, localization->str(LSTR_ZOOM_IN), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton(pViewport.topLeft() + Pointi(398, 38), {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_ZOOM_OUT), INPUT_ACTION_ZOOM_OUT, localization->str(LSTR_ZOOM_OUT), {ui_book_button2_on});
    pBtn_Book_3 = CreateButton(pViewport.topLeft() + Pointi(397, 113), {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_SCROLL_UP), INPUT_ACTION_DIALOG_UP, localization->str(LSTR_SCROLL_UP), {ui_book_button3_on});
    pBtn_Book_4 = CreateButton(pViewport.topLeft() + Pointi(397, 150), {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_SCROLL_DOWN), INPUT_ACTION_DIALOG_DOWN, localization->str(LSTR_SCROLL_DOWN), {ui_book_button4_on});
    pBtn_Book_5 = CreateButton(pViewport.topLeft() + Pointi(397, 188), {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_SCROLL_RIGHT), INPUT_ACTION_DIALOG_RIGHT, localization->str(LSTR_SCROLL_RIGHT), {ui_book_button5_on});
    pBtn_Book_6 = CreateButton(pViewport.topLeft() + Pointi(397, 226), {50, 34}, 1, 0, UIMSG_ClickBooksBtn,
         std::to_underlying(BOOK_SCROLL_LEFT), INPUT_ACTION_DIALOG_LEFT, localization->str(LSTR_SCROLL_LEFT), {ui_book_button6_on});
}

void GUIWindow_MapBook::Update() {
    render->DrawQuad2D(ui_exit_cancel_button_background, {471, 445});
    render->DrawQuad2D(ui_book_map_background, pViewport.topLeft());

    auto [minX, maxX] = viewparams->GetMapViewMinMaxX();
    auto [minY, maxY] = viewparams->GetMapViewMinMaxY();

    if ((_bookButtonClicked && _bookButtonAction == BOOK_ZOOM_IN) || viewparams->uMapBookMapZoom / 128 >= 12) {
        render->DrawQuad2D(ui_book_button1_off, pViewport.topLeft() + Pointi(408, 2));
    } else {
        render->DrawQuad2D(ui_book_button1_on, pViewport.topLeft() + Pointi(398, 1));
    }

    if ((_bookButtonClicked && _bookButtonAction == BOOK_ZOOM_OUT) || viewparams->uMapBookMapZoom / 128 <= 3) {
        render->DrawQuad2D(ui_book_button2_off, pViewport.topLeft() + Pointi(408, 38));
    } else {
        render->DrawQuad2D(ui_book_button2_on, pViewport.topLeft() + Pointi(398, 38));
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_SCROLL_UP || viewparams->sViewCenterY >= maxY) {
        render->DrawQuad2D(ui_book_button3_off, pViewport.topLeft() + Pointi(408, 113));
    } else {
        render->DrawQuad2D(ui_book_button3_on, pViewport.topLeft() + Pointi(398, 113));
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_SCROLL_DOWN || viewparams->sViewCenterY <= minY) {
        render->DrawQuad2D(ui_book_button4_off, pViewport.topLeft() + Pointi(408, 150));
    } else {
        render->DrawQuad2D(ui_book_button4_on, pViewport.topLeft() + Pointi(399, 150));
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_SCROLL_RIGHT || viewparams->sViewCenterX >= maxX) {
        render->DrawQuad2D(ui_book_button5_off, pViewport.topLeft() + Pointi(408, 188));
    } else {
        render->DrawQuad2D(ui_book_button5_on, pViewport.topLeft() + Pointi(397, 188));
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_SCROLL_LEFT || viewparams->sViewCenterX <= minX) {
        render->DrawQuad2D(ui_book_button6_off, pViewport.topLeft() + Pointi(408, 226));
    } else {
        render->DrawQuad2D(ui_book_button6_on, pViewport.topLeft() + Pointi(397, 226));
    }

    if (_bookButtonClicked == 10) {
        if (_bookButtonAction == BOOK_ZOOM_IN && viewparams->uMapBookMapZoom / 128 < 12) {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            viewparams->CenterOnPartyZoomIn();
        } else if (_bookButtonAction == BOOK_ZOOM_OUT && viewparams->uMapBookMapZoom / 128 > 3) {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            viewparams->CenterOnPartyZoomOut();
        } else if (_bookButtonAction == BOOK_SCROLL_UP && viewparams->sViewCenterY < maxY) {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            viewparams->MapViewUp();
        } else if (_bookButtonAction == BOOK_SCROLL_DOWN && viewparams->sViewCenterY > minY) {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            viewparams->MapViewDown();
        } else if (_bookButtonAction == BOOK_SCROLL_RIGHT && viewparams->sViewCenterX < maxX) {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            viewparams->MapViewRight();
        } else if (_bookButtonAction == BOOK_SCROLL_LEFT && viewparams->sViewCenterX > minX) {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            viewparams->MapViewLeft();
        }
    }

    if (_bookButtonClicked)
        _bookButtonClicked--;

    render->DrawQuad2D(ui_book_map_frame, {75, 22});
    DrawBook_Map_sub(97, 49, 361, 313);
    render->ResetUIClipRect();


    Recti map_window = pViewport;
    if (engine->_currentLoadedMapId != MAP_INVALID) {
        DrawTitleText(assets->pFontBookTitle.get(), -14, 12, ui_book_map_title_color, pMapStats->pInfos[engine->_currentLoadedMapId].name, 3, map_window);
    }

    auto party_coordinates = localization->format(LSTR_X_D_Y_D, static_cast<int>(pParty->pos.x), static_cast<int>(pParty->pos.y));

    map_window.x = 0;
    DrawTitleText(assets->pFontComic.get(), 0, 320, ui_book_map_coordinates_color, party_coordinates, 0, map_window);
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

        int loc_power = std::countr_zero(static_cast<unsigned int>(viewparams->location_minimap->width()));
        int scale_increment = (1 << (loc_power + 16)) / viewparams->uMapBookMapZoom;
        double MapSizeScale = (double)(1 << (16 - loc_power));
        int stepX_r_resets =
            (unsigned int)(int64_t)
            ((double)(center.x - maxPartyAxisDistance / (viewparams->uMapBookMapZoom / 384) + 32768) / MapSizeScale) << 16;
        int stepY_r = (int)(int64_t)
            ((double)(-center.y - maxPartyAxisDistance / (viewparams->uMapBookMapZoom / 384) + 32768) / MapSizeScale) << 16;
        int scaled_posY = stepY_r >> 16;

        static GraphicsImage *minimaptemp = nullptr;
        if (minimaptemp) {
            minimaptemp->release();
        }

        RgbaImage minimapImage = RgbaImage::solid(Color(), screenWidth, screenHeight);
        Color *minitempix = minimapImage.pixels().data();
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

        minimaptemp = GraphicsImage::Create(std::move(minimapImage));
        render->DrawQuad2D(minimaptemp, {tl_x, tl_y});
    } else {  // indoors
        if (!pIndoor->mapOutlines.empty()) {
            render->BeginLines2D();
            for (unsigned i = 0; i < pIndoor->mapOutlines.size(); ++i) {
                BLVMapOutline *pOutline = &pIndoor->mapOutlines[i];

                if (pIndoor->faces[pOutline->uFace1ID].Visible() &&
                    pIndoor->faces[pOutline->uFace2ID].Visible()) {
                    if (pIndoor->faces[pOutline->uFace1ID].attributes & FACE_SeenByParty ||
                        pIndoor->faces[pOutline->uFace2ID].attributes & FACE_SeenByParty) {
                        pOutline->uFlags = pOutline->uFlags | 1;
                        pIndoor->_visible_outlines[i >> 3] |= 1 << (7 - i % 8);

                        Vec2f Vert1 = (pIndoor->vertices[pOutline->uVertex1ID].xy() - center) / 65536.0f;
                        Vec2f Vert2 = (pIndoor->vertices[pOutline->uVertex2ID].xy() - center) / 65536.0f;

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

        render->DrawQuad2D(game_ui_minimap_dirs[ArrowOctant], {ArrowXPos, ArrowYPos}, colorTable.Red);
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
                    std::abs((int)model.boundingCenter.x - global_coord_X),
                    std::abs((int)model.boundingCenter.y - global_coord_Y),
                    0) < model.boundingRadius) {
                for (ODMFace &face : model.faces) {
                    if (face.eventId) {
                        if (!(face.attributes & FACE_HAS_EVENT)) {
                            std::string hintString = getEventHintString(face.eventId);
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
