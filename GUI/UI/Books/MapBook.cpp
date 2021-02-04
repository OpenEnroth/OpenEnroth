#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/Books/MapBook.h"
#include "GUI/UI/UIGame.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"


void DrawBook_Map_sub(unsigned int tl_x, unsigned int tl_y, unsigned int br_x, int br_y, int dummy);

Image *ui_book_map_background = nullptr;

GUIWindow_MapBook::GUIWindow_MapBook() : GUIWindow_Book() {
    this->ptr_1C = (void *)WINDOW_MapsBook;  // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

    pEventTimer->Pause();
    viewparams->sViewCenterX = pParty->vPosition.x;
    viewparams->sViewCenterY = pParty->vPosition.y;
    pAudioPlayer->StopChannels(-1, -1);
    pBooksButtonOverlay = new GUIWindow_BooksButtonOverlay(546, 353, 0, 0, pBtn_Maps);

    MapBookOpen = 1;

    ui_book_map_background = assets->GetImage_ColorKey("sbmap", 0x7FF);
    ui_book_button1_on = assets->GetImage_Alpha("zoom-on");
    ui_book_button2_on = assets->GetImage_Alpha("zoot-on");
    ui_book_button3_on = assets->GetImage_Alpha("tabNon");
    ui_book_button4_on = assets->GetImage_Alpha("tabSon");
    ui_book_button5_on = assets->GetImage_Alpha("tabEon");
    ui_book_button6_on = assets->GetImage_Alpha("tabWon");
    ui_book_button1_off = assets->GetImage_Alpha("zoom-off");
    ui_book_button2_off = assets->GetImage_Alpha("zoot-off");
    ui_book_button3_off = assets->GetImage_Alpha("tabNoff");
    ui_book_button4_off = assets->GetImage_Alpha("tabSoff");
    ui_book_button5_off = assets->GetImage_Alpha("tabEoff");
    ui_book_button6_off = assets->GetImage_Alpha("tabWoff");

    pBtn_Book_1 = CreateButton(
        pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 1, 50, 34, 1,
        0, UIMSG_ClickBooksBtn, 0, GameKey::None, localization->GetString(251),
        {{ui_book_button1_on}});  // "Zoom In"
    pBtn_Book_2 = CreateButton(
        pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 38, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 1, GameKey::None, localization->GetString(252),
        {{ui_book_button2_on}});  // "Zoom Out"
    pBtn_Book_3 = CreateButton(
        pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 113, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 2, GameKey::None,
        localization->GetString(192));  // Scroll Up
    pBtn_Book_4 = CreateButton(
        pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 150, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 3, GameKey::None,
        localization->GetString(193));  // Scroll Down
    pBtn_Book_5 = CreateButton(
        pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 188, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 4, GameKey::None,
        localization->GetString(573));  // "Scroll Right"
    pBtn_Book_6 = CreateButton(
        pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 226, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 5, GameKey::None,
        localization->GetString(572));  // "Scroll Left"
}

void GUIWindow_MapBook::Update() {
    render->DrawTextureAlphaNew(471 /  640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
    render->DrawTextureAlphaNew(pViewport->uViewportTL_X / 640.0f, pViewport->uViewportTL_Y / 480.0f, ui_book_map_background);

    if (BtnUp_flag || viewparams->uMapBookMapZoom / 128 >= 12)  // Button 1
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408) / 640.0f,
        (pViewport->uViewportTL_Y + 2) / 480.0f,
            ui_book_button1_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398) / 640.0f,
        (pViewport->uViewportTL_Y + 1) / 480.0f,
            ui_book_button1_on);

    if (BtnDown_flag || viewparams->uMapBookMapZoom / 128 <= 3)  // Button 2
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408) / 640.0f,
        (pViewport->uViewportTL_Y + 38) / 480.0f,
            ui_book_button2_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398) / 640.0f,
        (pViewport->uViewportTL_Y + 38) / 480.0f,
            ui_book_button2_on);

    if (Book_PageBtn3_flag)  // Button 3
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408) / 640.0f,
        (pViewport->uViewportTL_Y + 113) / 480.0f,
            ui_book_button3_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398) / 640.0f,
        (pViewport->uViewportTL_Y + 113) / 480.0f,
            ui_book_button3_on);

    if (Book_PageBtn4_flag)  // Button 4
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408) / 640.0f,
        (pViewport->uViewportTL_Y + 150) / 480.0f,
            ui_book_button4_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 399) / 640.0f,
        (pViewport->uViewportTL_Y + 150) / 480.0f,
            ui_book_button4_on);

    if (Book_PageBtn5_flag)  // Button 5
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408) / 640.0f,
        (pViewport->uViewportTL_Y + 188) / 480.0f,
            ui_book_button5_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 397) / 640.0f,
        (pViewport->uViewportTL_Y + 188) / 480.0f,
            ui_book_button5_on);

    if (Book_PageBtn6_flag)  // Button 6
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408) / 640.0f,
        (pViewport->uViewportTL_Y + 226) / 480.0f,
            ui_book_button6_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 397) / 640.0f,
        (pViewport->uViewportTL_Y + 226) / 480.0f,
            ui_book_button6_on);

    if (BtnDown_flag) viewparams->CenterOnPartyZoomIn();
    if (BtnUp_flag) viewparams->CenterOnPartyZoomOut();
    if (Book_PageBtn3_flag) viewparams->MapViewUp();
    if (Book_PageBtn4_flag) viewparams->MapViewDown();
    if (Book_PageBtn5_flag) viewparams->MapViewRight();
    if (Book_PageBtn6_flag) viewparams->MapViewLeft();

    if (BtnUp_flag | BtnDown_flag | Book_PageBtn3_flag | Book_PageBtn4_flag |
        Book_PageBtn5_flag | Book_PageBtn6_flag)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);

    BtnUp_flag = 0;
    BtnDown_flag = 0;
    Book_PageBtn6_flag = 0;
    Book_PageBtn5_flag = 0;
    Book_PageBtn4_flag = 0;
    Book_PageBtn3_flag = 0;

    render->DrawTextureAlphaNew(75 / 640.0f, 22 / 480.0f, ui_book_map_frame);
    DrawBook_Map_sub(97, 49, 361, 313, 0);
    render->ResetUIClipRect();

    GUIWindow map_window;
    map_window.uFrameWidth = game_viewport_width;
    map_window.uFrameHeight = game_viewport_height;
    map_window.uFrameX = game_viewport_x;
    map_window.uFrameY = game_viewport_y;
    map_window.uFrameZ = game_viewport_z;
    map_window.uFrameW = game_viewport_w;

    uint map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id)
        map_window.DrawTitleText(pBook2Font, -14, 12, ui_book_map_title_color, pMapStats->pInfos[map_id].pName, 3);

    auto party_coordinates = localization->FormatString(659, pParty->vPosition.x, pParty->vPosition.y);  // "x: %d  y: %d"

    map_window.uFrameX = 0;
    map_window.DrawTitleText(pFontComic, 0, 320, ui_book_map_coordinates_color, party_coordinates, 0);
}

//----- (00442955) --------------------------------------------------------
void DrawBook_Map_sub(unsigned int tl_x, unsigned int tl_y, unsigned int br_x, int br_y, int dummy) {
    int ScreenCenterX = (signed int)(tl_x + br_x) / 2;
    int ScreenCenterY = (signed int)(tl_y + br_y) / 2;
    render->SetUIClipRect(tl_x, tl_y, br_x, br_y);

    int pCenterX = viewparams->sViewCenterX;
    int pCenterY = viewparams->sViewCenterY;

    if (viewparams->uMapBookMapZoom == 384) {
        pCenterX = viewparams->indoor_center_x;
        pCenterY = viewparams->indoor_center_y;
    }

    if (uCurrentlyLoadedLevelType != LEVEL_Indoor) {  // outdoors
        int screenWidth = br_x - tl_x + 1;
        int screenHeight = br_y - tl_y + 1;

        int loc_power = ImageHelper::GetWidthLn2(viewparams->location_minimap);
        int scale_increment = (1 << (loc_power + 16)) / viewparams->uMapBookMapZoom;
        double MapSizeScale = (double)(1 << (16 - loc_power));
        int stepX_r_resets =
            (unsigned int)(signed __int64)
            ((double)(pCenterX - 22528 / (viewparams->uMapBookMapZoom / 384) + 32768) / MapSizeScale) << 16;
        int stepY_r = (int)(signed __int64)
            ((double)(-pCenterY - 22528 / (viewparams->uMapBookMapZoom / 384) + 32768) / MapSizeScale) << 16;
        int scaled_posY = stepY_r >> 16;

        static Texture *minimaptemp = nullptr;
        if (!minimaptemp) {
            minimaptemp = render->CreateTexture_Blank(screenWidth, screenHeight, IMAGE_FORMAT_A8R8G8B8);
        }
        auto minitempix = (unsigned __int32 *)minimaptemp->GetPixels(IMAGE_FORMAT_A8R8G8B8);
        auto minimap_pixels = (unsigned __int32 *)viewparams->location_minimap->GetPixels(IMAGE_FORMAT_A8R8G8B8);
        int textr_width = viewparams->location_minimap->GetWidth();

        // nearest neiborhood scaling
        // if (texture8_data)
        {
            for (uint i = 0; i < screenHeight; ++i) {
                int map_tile_Y = (scaled_posY - 80) / 4;
                int stepX_r = stepX_r_resets;
                for (uint j = 0; j < screenWidth; ++j) {
                    int scaled_posX = stepX_r >> 16;
                    int map_tile_X = (scaled_posX - 80) / 4;
                    if (!pOutdoor->IsMapCellFullyRevealed(map_tile_X, map_tile_Y)) {
                        if (pOutdoor->IsMapCellPartiallyRevealed(map_tile_X,
                            map_tile_Y)) {
                            if (!((i + ScreenCenterX + j) % 2))
                                minitempix[j + i * screenWidth] = Color32(12, 12, 12);  // greyed out
                            else
                                minitempix[j + i * screenWidth] = minimap_pixels[scaled_posX + scaled_posY * textr_width];
                        } else {
                            minitempix[j + i * screenWidth] = Color32(0, 0, 0);  // black
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
        render->DrawTextureAlphaNew(tl_x / 640., tl_y / 480., minimaptemp);
    } else {  // indoors
        unsigned int Colour_Black = Color16(0, 0, 0);

        if (pIndoor->pMapOutlines->uNumOutlines) {
            for (uint i = 0; i < pIndoor->pMapOutlines->uNumOutlines; ++i) {
                BLVMapOutline *pOutline = &pIndoor->pMapOutlines->pOutlines[i];

                if (pIndoor->pFaces[pOutline->uFace1ID].Visible() &&
                    pIndoor->pFaces[pOutline->uFace2ID].Visible()) {
                    if (pIndoor->pFaces[pOutline->uFace1ID].uAttributes & FACE_RENDERED ||
                        pIndoor->pFaces[pOutline->uFace2ID].uAttributes & FACE_RENDERED) {
                        pOutline->uFlags = pOutline->uFlags | 1;
                        pIndoor->_visible_outlines[i >> 3] |= 1 << (7 - i % 8);

                        int Vert1X = pIndoor->pVertices[pOutline->uVertex1ID].x - pCenterX;
                        int Vert2X = pIndoor->pVertices[pOutline->uVertex2ID].x - pCenterX;
                        int Vert1Y = pIndoor->pVertices[pOutline->uVertex1ID].y - pCenterY;
                        int Vert2Y = pIndoor->pVertices[pOutline->uVertex2ID].y - pCenterY;

                        int linex = ScreenCenterX + fixpoint_mul(Vert1X, viewparams->uMapBookMapZoom);
                        int liney = ScreenCenterY - fixpoint_mul(Vert1Y, viewparams->uMapBookMapZoom);
                        int linez = ScreenCenterX + fixpoint_mul(Vert2X, viewparams->uMapBookMapZoom);
                        int linew = ScreenCenterY - fixpoint_mul(Vert2Y, viewparams->uMapBookMapZoom);

                        render->RasterLine2D(linex, liney, linez, linew, Colour_Black);
                    }
                }
            }
        }
    }

    // Direction arrow drawing
    int ArrowXPos = (fixpoint_mul((pParty->vPosition.x - pCenterX), viewparams->uMapBookMapZoom)) + ScreenCenterX - 3;
    int ArrowYPos = ScreenCenterY - (fixpoint_mul((pParty->vPosition.y - pCenterY), viewparams->uMapBookMapZoom)) - 3;
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
        int PartyDirection = pParty->sRotationZ & TrigLUT->uDoublePiMask;
        if ((signed int)PartyDirection <= 1920) ArrowOctant = 6;
        if ((signed int)PartyDirection < 1664) ArrowOctant = 5;
        if ((signed int)PartyDirection <= 1408) ArrowOctant = 4;
        if ((signed int)PartyDirection < 1152) ArrowOctant = 3;
        if ((signed int)PartyDirection <= 896) ArrowOctant = 2;
        if ((signed int)PartyDirection < 640) ArrowOctant = 1;
        if ((signed int)PartyDirection <= 384) ArrowOctant = 0;
        if ((signed int)PartyDirection < 128 || (signed int)PartyDirection > 1920) ArrowOctant = 7;

        render->DrawTransparentRedShade(ArrowXPos / 640.0f, ArrowYPos / 480.0f, game_ui_minimap_dirs[ArrowOctant]);
    }

    if ((signed int)uNumLevelDecorations > 0) {
        for (uint i = 0; i < (signed int)uNumLevelDecorations; ++i) {
            if (pLevelDecorations[i].uFlags & LEVEL_DECORATION_VISIBLE_ON_MAP) {
                int DecY = pLevelDecorations[i].vPosition.y - pCenterY;
                int DecX = pLevelDecorations[i].vPosition.x - pCenterX;
                int decxpos = ScreenCenterX + fixpoint_mul(DecX, viewparams->uMapBookMapZoom);
                int decypos = ScreenCenterY - fixpoint_mul(DecY, viewparams->uMapBookMapZoom);

                if (viewparams->uMapBookMapZoom > 512) {
                    render->RasterLine2D(decxpos - 1, decypos - 1, decxpos - 1, decypos + 1, Color16(0xFFu, 0xFFu, 0xFFu));
                    render->RasterLine2D(decxpos, decypos - 1, decxpos, decypos + 1, Color16(0xFFu, 0xFFu, 0xFFu));
                    render->RasterLine2D(decxpos + 1, decypos - 1, decxpos + 1, decypos + 1, Color16(0xFFu, 0xFFu, 0xFFu));
                } else {
                    render->RasterLine2D(decxpos, decypos, decxpos, decypos, Color16(0xFF, 0xFF, 0xFF));
                }
            }
        }
    }
}

const char *GetMapBookHintText(int mouse_x, int mouse_y) {
    int v20 = viewparams->sViewCenterX;
    int v21 = viewparams->sViewCenterY;
    if (viewparams->uMapBookMapZoom == 384) {
        v20 = viewparams->indoor_center_x;
        v21 = viewparams->indoor_center_y;
    }

    unsigned int pX = mouse_x;
    unsigned int pY = mouse_y;

    double v0 = 1.0 / (float)((signed int)viewparams->uMapBookMapZoom * 0.000015258789);

    int global_coord_X = (__int64)((double)(pX - 229.) * v0 + (double)v20);
    int global_coord_Y = (__int64)((double)v21 - (double)(pY - 181.) * v0);

    const char *result = "";

    // In the mapbook only lady Margaret dispays for defoult zoom(В
    // книге карты только Леди Маргарита всплывает при дефолтном зуме)
    int map_tile_X = abs(global_coord_X + 22528) / 512;
    int map_tile_Y = abs(global_coord_Y - 22528) / 512;
    if (pOutdoor->IsMapCellFullyRevealed(map_tile_X, map_tile_Y) &&
        uCurrentlyLoadedLevelType == LEVEL_Outdoor &&
        !pOutdoor->pBModels.empty()) {
        for (BSPModel &model : pOutdoor->pBModels) {
            if (int_get_vector_length(
                    abs((int)model.vBoundingCenter.x - global_coord_X),
                    abs((int)model.vBoundingCenter.y - global_coord_Y),
                    0) < model.sBoundingRadius) {
                for (ODMFace &face : model.pFaces) {
                    if (face.sCogTriggeredID) {
                        if (!(face.uAttributes & FACE_HAS_EVENT)) {
                            if (GetEventHintString(face.sCogTriggeredID)) {
                                if (_stricmp(GetEventHintString(face.sCogTriggeredID), "")) {
                                    result = GetEventHintString(face.sCogTriggeredID);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}
