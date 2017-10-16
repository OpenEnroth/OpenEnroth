#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Party.h"
#include "Engine/Time.h"
#include "Engine/Events.h"
#include "Engine/OurMath.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Level/Decoration.h"

#include "IO/Mouse.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/Books/MapBook.h"

#include "Media/Audio/AudioPlayer.h"

void DrawBook_Map_sub(unsigned int tl_x, unsigned int tl_y, unsigned int br_x, int br_y, int _48074); // idb




Image *ui_book_map_background = nullptr;



GUIWindow_MapBook::GUIWindow_MapBook() :
    GUIWindow_Book()
{
    this->ptr_1C = (void *)WINDOW_MapsBook; // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

// ----------------------------------------------
// 00411BFC GUIWindow::InitializeBookView -- part
    pEventTimer->Pause();
    viewparams->sViewCenterX = pParty->vPosition.x;
    viewparams->sViewCenterY = pParty->vPosition.y;
    pAudioPlayer->StopChannels(-1, -1);
    pBooksButtonOverlay = new GUIWindow_BooksButtonOverlay(546, 353, 0, 0, (int)pBtn_Maps, 0);

// ----------------------------------------------
// 00411BFC GUIWindow::InitializeBookView -- part
    dword_506364 = 1;
    ui_book_map_background = assets->GetImage_16BitColorKey("sbmap", 0x7FF);

    ui_book_button1_on = assets->GetImage_16BitAlpha("zoom-on");
    ui_book_button2_on = assets->GetImage_16BitAlpha("zoot-on");
    ui_book_button3_on = assets->GetImage_16BitAlpha("tabNon");
    ui_book_button4_on = assets->GetImage_16BitAlpha("tabSon");
    ui_book_button5_on = assets->GetImage_16BitAlpha("tabEon");
    ui_book_button6_on = assets->GetImage_16BitAlpha("tabWon");
    ui_book_button1_off = assets->GetImage_16BitAlpha("zoom-off");
    ui_book_button2_off = assets->GetImage_16BitAlpha("zoot-off");
    ui_book_button3_off = assets->GetImage_16BitAlpha("tabNoff");
    ui_book_button4_off = assets->GetImage_16BitAlpha("tabSoff");
    ui_book_button5_off = assets->GetImage_16BitAlpha("tabEoff");
    ui_book_button6_off = assets->GetImage_16BitAlpha("tabWoff");

    pBtn_Book_1 = this->CreateButton(pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 1, 50, 34, 1, 0,
        UIMSG_ClickBooksBtn, 0, 0, localization->GetString(251), ui_book_button1_on, 0);// "Zoom In"
    pBtn_Book_2 = this->CreateButton(pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 38, 50, 34, 1, 0,
        UIMSG_ClickBooksBtn, 1, 0, localization->GetString(252), ui_book_button2_on, 0);// "Zoom Out"
    pBtn_Book_3 = this->CreateButton(pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 113, 50, 34, 1, 0,
        UIMSG_ClickBooksBtn, 2, 0, localization->GetString(192), (Image *)"", 0);// Scroll Up
    pBtn_Book_4 = this->CreateButton(pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 150, 50, 34, 1, 0,
        UIMSG_ClickBooksBtn, 3, 0, localization->GetString(193), (Image *)"", 0);// Scroll Down
    pBtn_Book_5 = this->CreateButton(pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 188, 50, 34, 1, 0,
        UIMSG_ClickBooksBtn, 4, 0, localization->GetString(573), (Image *)"", 0);// "Scroll Right"
    pBtn_Book_6 = this->CreateButton(pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 226, 50, 34, 1, 0,
        UIMSG_ClickBooksBtn, 5, 0, localization->GetString(572), (Image *)"", 0);// "Scroll Left"
}




void GUIWindow_MapBook::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
// {
//     BookUI_Draw((WindowType)(int)ptr_1C);
// }

// ----- (00413CC6) --------------------------------------------------------
// void BookUI_Draw(WindowType book) --- part
// {
    render->DrawTextureAlphaNew(471/640.0f, 445/480.0f, ui_exit_cancel_button_background);
//     BookUI_Map_Draw();

// ----- (00413980) --------------------------------------------------------
// void BookUI_Map_Draw()
// {
    unsigned int map_id; // eax@35
    //char party_coord[120]; // [sp+Ch] [bp-CCh]@37
    GUIWindow map_window; // [sp+84h] [bp-54h]@35

    render->DrawTextureAlphaNew(pViewport->uViewportTL_X/640.0f, pViewport->uViewportTL_Y/480.0f, ui_book_map_background);
    if (BtnUp_flag || viewparams->uMapBookMapZoom / 128 >= 12)//Button 1
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408)/640.0f, (pViewport->uViewportTL_Y + 2)/480.0f, ui_book_button1_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398)/640.0f, (pViewport->uViewportTL_Y + 1)/480.0f, ui_book_button1_on);

    if (BtnDown_flag || viewparams->uMapBookMapZoom / 128 <= 3)//Button 2
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408)/640.0f, (pViewport->uViewportTL_Y + 38)/480.0f, ui_book_button2_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398)/640.0f, (pViewport->uViewportTL_Y + 38)/480.0f, ui_book_button2_on);

    if (Book_PageBtn3_flag)//Button 3
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408)/640.0f, (pViewport->uViewportTL_Y + 113)/480.0f, ui_book_button3_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398)/640.0f, (pViewport->uViewportTL_Y + 113)/480.0f, ui_book_button3_on);

    if (Book_PageBtn4_flag)//Button 4
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408)/640.0f, (pViewport->uViewportTL_Y + 150)/480.0f, ui_book_button4_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 399)/640.0f, (pViewport->uViewportTL_Y + 150)/480.0f, ui_book_button4_on);

    if (Book_PageBtn5_flag)//Button 5
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408)/640.0f, (pViewport->uViewportTL_Y + 188)/480.0f, ui_book_button5_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 397)/640.0f, (pViewport->uViewportTL_Y + 188)/480.0f, ui_book_button5_on);

    if (Book_PageBtn6_flag)//Button 6
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 408)/640.0f, (pViewport->uViewportTL_Y + 226)/480.0f, ui_book_button6_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 397)/640.0f, (pViewport->uViewportTL_Y + 226)/480.0f, ui_book_button6_on);

    if (BtnDown_flag)
        viewparams->CenterOnParty2();
    if (BtnUp_flag)
        viewparams->CenterOnParty();
    if (Book_PageBtn3_flag)
        viewparams->_443219();
    if (Book_PageBtn4_flag)
        viewparams->_443231();
    if (Book_PageBtn5_flag)
        viewparams->_44323D();
    if (Book_PageBtn6_flag)
        viewparams->_443225();

    if (BtnUp_flag | BtnDown_flag | Book_PageBtn3_flag | Book_PageBtn4_flag | Book_PageBtn5_flag | Book_PageBtn6_flag)
        pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);
    BtnUp_flag = 0;
    BtnDown_flag = 0;
    Book_PageBtn6_flag = 0;
    Book_PageBtn5_flag = 0;
    Book_PageBtn4_flag = 0;
    Book_PageBtn3_flag = 0;
    DrawBook_Map_sub(97, 49, 361, 313, 0);
    render->DrawTextureAlphaNew(75/640.0f, 22/480.0f, ui_book_map_frame);
    map_window.uFrameWidth = game_viewport_width;
    map_window.uFrameHeight = game_viewport_height;
    map_window.uFrameX = game_viewport_x;
    map_window.uFrameY = game_viewport_y;
    map_window.uFrameZ = game_viewport_z;
    map_window.uFrameW = game_viewport_w;
    map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id)
        map_window.DrawTitleText(pBook2Font, -14, 12, ui_book_map_title_color, pMapStats->pInfos[map_id].pName, 3);

    auto party_coordinates = localization->FormatString(659, pParty->vPosition.x, pParty->vPosition.y); //"x: %d  y: %d"

    map_window.uFrameX = 0;
    map_window.DrawTitleText(pFontComic, 0, 320, ui_book_map_coordinates_color, party_coordinates, 0);
}












//----- (00442955) --------------------------------------------------------
void DrawBook_Map_sub(unsigned int tl_x, unsigned int tl_y, unsigned int br_x, int br_y, int _48074)
{
    int v20; // eax@16
    int v26; // ecx@21
    unsigned __int16 *v27; // edi@21
    int v28; // edx@21
    int v29; // eax@21
    double v30; // st7@23
    int textr_width; // esi@23
    signed int v47; // esi@38
    signed int v48; // ecx@38
    int v49; // eax@38
    signed int v50; // edx@55
    unsigned int v51; // ecx@55
    int v54; // esi@75
    int v55; // eax@75
    double v57; // st7@85
    signed int v60; // edi@85
    signed int v62; // ebx@85
    signed int v63; // esi@85
    int v64; // eax@87
    //unsigned __int16 *pPalette_16; // [sp+48014h] [bp-50h]@23
    int map_tile_X; // [sp+48020h] [bp-44h]@23
    //unsigned char* texture8_data;
    int scale_increment;
    int scaled_posX;
    int scaled_posY;
    int stepX_r;
    int stepY_r;
    unsigned int teal; // [sp+48028h] [bp-3Ch]@8
    int pCenterY; // [sp+4802Ch] [bp-38h]@1
    int screenCenter_X; // [sp+48030h] [bp-34h]@1
    int pCenterX; // [sp+48034h] [bp-30h]@1
    int v87; // [sp+48038h] [bp-2Ch]@16
    unsigned int v88; // [sp+4803Ch] [bp-28h]@16
    int black; // [sp+48040h] [bp-24h]@8
    int screenCenterY; // [sp+48044h] [bp-20h]@1
    unsigned int screenHeight; // [sp+4804Ch] [bp-18h]@16
    unsigned __int16 *v93; // [sp+48050h] [bp-14h]@16
    signed int screenWidth; // [sp+48054h] [bp-10h]@8
    unsigned int v95; // [sp+48058h] [bp-Ch]@16
    int map_tile_Y; // [sp+4805Ch] [bp-8h]@10
    const void *v97; // [sp+48060h] [bp-4h]@16

    screenCenter_X = (signed int)(tl_x + br_x) / 2;
    screenCenterY = (signed int)(tl_y + br_y) / 2;
    render->SetRasterClipRect(tl_x, tl_y, br_x, br_y);
    pCenterX = viewparams->sViewCenterX;
    pCenterY = viewparams->sViewCenterY;
    if (viewparams->uMapBookMapZoom != 384)
    {
        if (viewparams->uMapBookMapZoom == 768)
        {
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                viewparams->uMapBookMapZoom = 680;
        }
    }
    else
    {
        viewparams->sViewCenterX = viewparams->indoor_center_x;
        pCenterX = viewparams->indoor_center_x;
        pCenterY = viewparams->indoor_center_y;
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
            viewparams->uMapBookMapZoom = viewparams->uMapBookMapZoom - 34;
    }
    if (uCurrentlyLoadedLevelType != LEVEL_Indoor)
    {
        screenWidth = br_x - tl_x + 1;
        screenHeight = br_y - tl_y + 1;

        scale_increment = (1 << (ImageHelper::GetWidthLn2(viewparams->location_minimap) + 16)) / viewparams->uMapBookMapZoom;

        v30 = (double)(1 << (16 - ImageHelper::GetWidthLn2(viewparams->location_minimap)));

        teal = (unsigned int)(signed __int64)((double)(viewparams->sViewCenterX - 22528 / (viewparams->uMapBookMapZoom / 384) + 32768) / v30) << 16;

        textr_width = viewparams->location_minimap->GetWidth();
        stepY_r = (int)(signed __int64)((double)(-pCenterY - 22528 / (viewparams->uMapBookMapZoom / 384) + 32768) / v30) << 16;
        black = (signed __int16)(signed __int64)((double)(viewparams->sViewCenterX - 22528 / (viewparams->uMapBookMapZoom / 384) + 32768) / v30);

        scaled_posY = stepY_r >> 16;

        auto minimap_pixels = (unsigned __int16 *)viewparams->location_minimap->GetPixels(IMAGE_FORMAT_R5G6B5);

        //nearest neiborhood scaling
        //if (texture8_data)
        {
            for (uint i = 0; i < screenHeight; ++i)
            {
                unsigned __int16 *curr_line = &minimap_pixels[scaled_posY*textr_width];
                stepX_r = teal;
                for (uint j = 0; j < screenWidth; ++j)
                {
                    scaled_posX = stepX_r >> 16;
                    //map_texture_16[i*screenWidth+j]=pPalette_16[*(curr_line+scaled_posX)];
                    render->WritePixel16(tl_x + j, tl_y + i, *(curr_line + scaled_posX));
                    stepX_r += scale_increment;
                }
                stepY_r += scale_increment;
                scaled_posY = stepY_r >> 16;
            }
        }

        /*//move visible square to render
        for( uint i = 0; i < screenHeight; ++i )
        {
        if ( screenWidth > 0 )
        memcpy((void*)&render16_data[render->uTargetSurfacePitch * i],(void*)&map_texture_16[i*screenWidth], screenWidth*2);
        }*/
    }
    else
    {
        black = Color16(0, 0, 0);
        teal = Color16(0, 0xFF, 0xFF);
        uNumBlueFacesInBLVMinimap = 0;
        if (pIndoor->pMapOutlines->uNumOutlines)
        {
            for (uint i = 0; i < pIndoor->pMapOutlines->uNumOutlines; ++i)
            {
                if (!(pIndoor->pFaces[pIndoor->pMapOutlines->pOutlines[i].uFace1ID].Invisible()
                    || (pIndoor->pFaces[pIndoor->pMapOutlines->pOutlines[i].uFace2ID].Invisible())))
                {
                    if (!(pIndoor->pMapOutlines->pOutlines[i].uFlags & 1))
                    {
                        if (!(!(pIndoor->pFaces[pIndoor->pMapOutlines->pOutlines[i].uFace1ID].uAttributes & FACE_UNKNOW7)
                            && !(pIndoor->pFaces[pIndoor->pMapOutlines->pOutlines[i].uFace2ID].uAttributes & FACE_UNKNOW7)))
                        {
                            pIndoor->pMapOutlines->pOutlines[i].uFlags = pIndoor->pMapOutlines->pOutlines[i].uFlags | 1;
                            pIndoor->_visible_outlines[i >> 3] |= 1 << (7 - i % 8);
                        }
                    }
                    if ((!(pIndoor->pMapOutlines->pOutlines[i].uFlags & 1)
                        && !(!(pIndoor->pFaces[pIndoor->pMapOutlines->pOutlines[i].uFace1ID].uAttributes & FACE_UNKNOW7)
                        && !(pIndoor->pFaces[pIndoor->pMapOutlines->pOutlines[i].uFace2ID].uAttributes & FACE_UNKNOW7)))
                        || pIndoor->pMapOutlines->pOutlines[i].uFlags & 1)
                    {
                        v93 = (unsigned __int16 *)(pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex1ID].x - viewparams->sViewCenterX);
                        screenHeight = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex1ID].y - pCenterY;
                        v20 = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].y - pCenterY;
                        v95 = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].x - viewparams->sViewCenterX;

                        v88 = fixpoint_mul((pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex1ID].x - viewparams->sViewCenterX), viewparams->uMapBookMapZoom);
                        v87 = fixpoint_mul(screenHeight, viewparams->uMapBookMapZoom);
                        v93 = (unsigned __int16 *)(fixpoint_mul((pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].x - viewparams->sViewCenterX), viewparams->uMapBookMapZoom));
                        screenHeight = fixpoint_mul(v20, viewparams->uMapBookMapZoom);
                        render->RasterLine2D(screenCenter_X + v88, screenCenterY - v87,
                            screenCenter_X + (fixpoint_mul((pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].x - viewparams->sViewCenterX), viewparams->uMapBookMapZoom)), screenCenterY - screenHeight, black);
                    }
                }
            }
        }
        if ((signed int)uNumBlueFacesInBLVMinimap > 0)
        {
            for (uint j = 0; j < (signed int)uNumBlueFacesInBLVMinimap; ++j)
            {
                v26 = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[pBlueFacesInBLVMinimapIDs[j]].uVertex2ID].x;
                v27 = (unsigned __int16 *)(pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[pBlueFacesInBLVMinimapIDs[j]].uVertex1ID].x - pCenterX);
                v28 = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[pBlueFacesInBLVMinimapIDs[j]].uVertex1ID].y - pCenterY;
                v29 = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[pBlueFacesInBLVMinimapIDs[j]].uVertex2ID].y - pCenterY;

                v87 = fixpoint_mul((signed int)v27, viewparams->uMapBookMapZoom);
                v88 = fixpoint_mul(v28, viewparams->uMapBookMapZoom);
                uint i = fixpoint_mul((v26 - pCenterX), viewparams->uMapBookMapZoom);
                v95 = fixpoint_mul(v29, viewparams->uMapBookMapZoom);
                render->RasterLine2D(screenCenter_X + (fixpoint_mul((signed int)v27, viewparams->uMapBookMapZoom)),
                    screenCenterY - v88, screenCenter_X + (fixpoint_mul((v26 - pCenterX), viewparams->uMapBookMapZoom)), screenCenterY - v95, teal);
            }
            viewparams->sViewCenterX = pCenterX;
        }
    }
    v47 = (fixpoint_mul((pParty->vPosition.x - viewparams->sViewCenterX), viewparams->uMapBookMapZoom)) + screenCenter_X - 3;
    v97 = (const void *)(fixpoint_mul((pParty->vPosition.y - pCenterY), viewparams->uMapBookMapZoom));
    v48 = 1;
    v49 = screenCenterY - (int)v97 - 3;
    if (v47 >= (signed int)tl_x)
    {
        if (v47 > (signed int)br_x)
        {
            if ((signed int)((fixpoint_mul((pParty->vPosition.x - viewparams->sViewCenterX), viewparams->uMapBookMapZoom)) + screenCenter_X - 6) > (signed int)br_x)
                v48 = 0;
            v47 = br_x;
        }
    }
    else
    {
        if ((signed int)((fixpoint_mul((pParty->vPosition.x - viewparams->sViewCenterX), viewparams->uMapBookMapZoom)) + screenCenter_X) < (signed int)tl_x)
            v48 = 0;
        v47 = tl_x;
    }
    if (v49 >= (signed int)tl_y)
    {
        if (v49 > br_y)
        {
            if (screenCenterY - (signed int)v97 - 6 > br_y)
                v48 = 0;
            v49 = br_y;
        }
    }
    else
    {
        if (screenCenterY - (signed int)v97 < (signed int)tl_y)
            v48 = 0;
        v49 = tl_y;
    }
    if (v48 == 1)
    {
        v51 = pParty->sRotationY & stru_5C6E00->uDoublePiMask;
        if ((signed int)v51 <= 1920)
            v50 = 6;
        if ((signed int)v51 < 1664)
            v50 = 5;
        if ((signed int)v51 <= 1408)
            v50 = 4;
        if ((signed int)v51 < 1152)
            v50 = 3;
        if ((signed int)v51 <= 896)
            v50 = 2;
        if ((signed int)v51 < 640)
            v50 = 1;
        if ((signed int)v51 <= 384)
            v50 = 0;
        if ((signed int)v51 < 128 || (signed int)v51 > 1920)
            v50 = 7;
        render->DrawTransparentRedShade(v47/640.0f, v49/480.0f, game_ui_minimap_dirs[v50]);
    }
    if ((signed int)uNumLevelDecorations > 0)
    {
        for (uint i = 0; i < (signed int)uNumLevelDecorations; ++i)
        {
            if (pLevelDecorations[i].uFlags & LEVEL_DECORATION_VISIBLE_ON_MAP)
            {
                screenHeight = pLevelDecorations[i].vPosition.y - pCenterY;
                v93 = (unsigned __int16 *)(pLevelDecorations[i].vPosition.x - viewparams->sViewCenterX);
                v54 = (fixpoint_mul((signed int)v93, viewparams->uMapBookMapZoom)) + screenCenter_X;
                v97 = (const void *)(fixpoint_mul(screenHeight, viewparams->uMapBookMapZoom));
                v55 = screenCenterY - (int)v97;
                //if ( v54 >= render->raster_clip_x && v54 <= render->raster_clip_z
                //  && v55 >= render->raster_clip_y && v55 <= render->raster_clip_w )
                {
                    if (viewparams->uMapBookMapZoom > 512)
                    {
                        render->RasterLine2D(v54 - 1, v55 - 1, v54 - 1, v55 + 1, Color16(0xFFu, 0xFFu, 0xFFu));
                        render->RasterLine2D(v54, v55 - 1, v54, v55 + 1, Color16(0xFFu, 0xFFu, 0xFFu));
                        render->RasterLine2D(v54 + 1, v55 - 1, v54 + 1, v55 + 1, Color16(0xFFu, 0xFFu, 0xFFu));
                    }
                    else
                        render->RasterLine2D(v54, screenCenterY - (int)v97,
                        (fixpoint_mul((signed int)v93, viewparams->uMapBookMapZoom)) + screenCenter_X,
                        screenCenterY - (int)v97, Color16(0xFF, 0xFF, 0xFF));
                }
            }
        }
    }
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
    {
        screenCenterY = br_x - tl_x + 1;
        v95 = br_y - tl_y + 1;
        //v77 = &render->pTargetSurface[tl_x + tl_y * render->uTargetSurfacePitch];
        black = (1 << (ImageHelper::GetWidthLn2(viewparams->location_minimap) + 16)) / viewparams->uMapBookMapZoom;
        v57 = (double)(1 << (16 - ImageHelper::GetWidthLn2(viewparams->location_minimap)));
        v60 = (int)((signed __int64)((double)(viewparams->sViewCenterX - (22528 / (viewparams->uMapBookMapZoom / 384)) + 32768) / v57)) << 16;
        teal = v60 >> 16;
        v97 = (const void *)((int)((signed __int64)((double)(viewparams->sViewCenterX - (22528 / (viewparams->uMapBookMapZoom / 384)) + 32768) / v57)) << 16);
        v62 = (int)((signed __int64)((double)(32768 - (22528 / (viewparams->uMapBookMapZoom / 384)) - pCenterY) / v57)) << 16;
        v63 = (signed __int16)((signed __int64)((double)(32768 - (22528 / (viewparams->uMapBookMapZoom / 384)) - pCenterY) / v57));


        for (int y = 0; y < (signed int)v95; ++y)
        {
            map_tile_Y = (v63 - 80) / 4;
            v64 = teal;
            for (int x = 0; x < screenCenterY; ++x)
            {
                map_tile_X = (v64 - 80) / 4;
                if (!pOutdoor->IsMapCellFullyRevealed(map_tile_X, map_tile_Y))
                {
                    if (pOutdoor->IsMapCellPartiallyRevealed(map_tile_X, map_tile_Y))
                    {
                        if (!((x + screenCenter_X) % 2))
                            //*a4a = Color16(12, 12, 12);
                            render->WritePixel16(tl_x + x, tl_y + y, Color16(12, 12, 12));
                    }
                    else
                        //*a4a = 0;
                        render->WritePixel16(tl_x + x, tl_y + y, Color16(0, 0, 0));
                }
                v97 = (char *)v97 + black;
                v64 = (signed int)v97 >> 16;
            }

            v62 += black;
            v97 = (const void *)v60;
            v63 = v62 >> 16;
        }
    }
}

//----- (00444564) --------------------------------------------------------
const char * GetMapBookHintText()
{
    int v20;
    int v21; // [sp+14h] [bp-Ch]@1
    double v0; // st7@3
    unsigned int pX; // [sp+1Ch] [bp-4h]@3
    unsigned int pY; // [sp+8h] [bp-18h]@3
    int global_coord_X; // ebx@3
    int global_coord_Y;
    int map_tile_X; // edi@3
    int map_tile_Y; // eax@3
    const char *result; // eax@15

    v20 = viewparams->sViewCenterX;
    v21 = viewparams->sViewCenterY;
    if (viewparams->uMapBookMapZoom == 384)
    {
        v20 = viewparams->indoor_center_x;
        v21 = viewparams->indoor_center_y;
    }
    pMouse->GetClickPos(&pX, &pY);
    v0 = 1.0 / (float)((signed int)viewparams->uMapBookMapZoom * 0.000015258789);

    global_coord_X = (signed __int64)((double)(pX - 229) * v0 + (double)v20);
    global_coord_Y = (signed __int64)((double)v21 - (double)(pY - 181) * v0);

    result = 0;
    map_tile_X = abs(global_coord_X + 22528) / 512;//In the mapbook only lady Margaret dispays for defoult zoom(В книге карты только Леди Маргарита всплывает при дефолтном зуме)
    map_tile_Y = abs(global_coord_Y - 22528) / 512;
    if (pOutdoor->IsMapCellFullyRevealed(map_tile_X, map_tile_Y) && uCurrentlyLoadedLevelType == LEVEL_Outdoor && (signed int)pOutdoor->uNumBModels > 0)
    {
        for (int i = 0; i < pOutdoor->uNumBModels && !result; i++)
        {
            if (int_get_vector_length(abs((signed)pOutdoor->pBModels[i].vBoundingCenter.x - global_coord_X),
                abs((signed)pOutdoor->pBModels[i].vBoundingCenter.y - global_coord_Y), 0) < pOutdoor->pBModels[i].sBoundingRadius)
            {
                if (pOutdoor->pBModels[i].uNumFaces > 0)
                {
                    for (int j = 0; j < pOutdoor->pBModels[i].uNumFaces; j++)
                    {
                        if (pOutdoor->pBModels[i].pFaces[j].sCogTriggeredID)
                        {
                            if (!(pOutdoor->pBModels[i].pFaces[j].uAttributes & FACE_HAS_EVENT))
                            {
                                if (GetEventHintString(pOutdoor->pBModels[i].pFaces[j].sCogTriggeredID))
                                {
                                    if (_stricmp(GetEventHintString(pOutdoor->pBModels[i].pFaces[j].sCogTriggeredID), ""))
                                        result = GetEventHintString(pOutdoor->pBModels[i].pFaces[j].sCogTriggeredID);
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