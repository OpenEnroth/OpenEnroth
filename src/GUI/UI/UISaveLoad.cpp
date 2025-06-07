#include "GUI/UI/UISaveLoad.h"

#include <string>
#include <algorithm>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Snapshots/EntitySnapshots.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/SaveLoad.h"

#include "Media/Audio/AudioPlayer.h"

#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"

#include "Library/Lod/LodReader.h"
#include "Library/Logger/Logger.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/String/Ascii.h"

using Io::TextInputType;

static void UI_DrawSaveLoad(bool save);

std::array<int, 2> saveload_dlg_xs = {{82, 0}};
std::array<int, 2> saveload_dlg_ys = {{60, 0}};
std::array<int, 2> saveload_dlg_zs = {{460, 640}};
std::array<int, 2> saveload_dlg_ws = {{344, 480}};

GraphicsImage *saveload_ui_ls_saved = nullptr;
GraphicsImage *saveload_ui_x_d = nullptr;

static GraphicsImage *scrollstop = nullptr;

GUIWindow_Save::GUIWindow_Save() : GUIWindow(WINDOW_Save, {0, 0}, render->GetRenderDimensions()) {
    saveload_ui_loadsave = assets->getImage_ColorKey("loadsave");
    saveload_ui_save_up = assets->getImage_ColorKey("save_up");
    saveload_ui_saveu = assets->getImage_ColorKey("LS_saveU");
    saveload_ui_x_u = assets->getImage_ColorKey("x_u");

    pSavegameList->Initialize();

    LodReader pLODFile;
    for (int i = 0; i < MAX_SAVE_SLOTS; ++i) {
        // std::string file_name = pSavegameList->pFileList[i];
        std::string file_name = fmt::format("save{:03}.mm7", i);
        if (file_name.empty()) {
            file_name = "1.mm7";
        }

        std::string str = fmt::format("saves/{}", file_name);
        if (!ufs->exists(str)) {
            pSavegameList->pSavegameUsedSlots[i] = false;
            pSavegameList->pSavegameHeader[i].name = localization->GetString(LSTR_EMPTY_SAVE);
        } else {
            pLODFile.open(ufs->read(str), LOD_ALLOW_DUPLICATES);
            deserialize(pLODFile.read("header.bin"), &pSavegameList->pSavegameHeader[i], tags::via<SaveGameHeader_MM7>);

            if (pSavegameList->pSavegameHeader[i].name.empty()) {
                // blank so add something - suspect quicksaves
                std::string newname = pSavegameList->pFileList[i];
                std::string test = newname.substr(0, newname.size() - 4);
                pSavegameList->pSavegameHeader[i].name = test;
            }

            pSavegameList->pSavegameThumbnails[i] = GraphicsImage::Create(std::make_unique<PCX_LOD_Raw_Loader>(&pLODFile, "image.pcx"));
            if (pSavegameList->pSavegameThumbnails[i]->width() == 0) {
                pSavegameList->pSavegameThumbnails[i]->Release();
                pSavegameList->pSavegameThumbnails[i] = nullptr;
            }

            pSavegameList->pSavegameUsedSlots[i] = (pSavegameList->pSavegameThumbnails[i] != nullptr);
        }
    }

    saveload_ui_x_d = assets->getImage_ColorKey("x_d");
    saveload_ui_ls_saved = assets->getImage_ColorKey("LS_saveD");
    ui_ar_up_dn = assets->getImage_ColorKey("ar_up_dn");
    ui_ar_dn_dn = assets->getImage_ColorKey("ar_dn_dn");

    scrollstop = assets->getImage_ColorKey("con_x");

    // GUIWindow_Save c-tor --- part
    CreateButton("SaveMenu_Slot0", {21, 198}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 0);
    CreateButton("SaveMenu_Slot1", {21, 218}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 1);
    CreateButton("SaveMenu_Slot2", {21, 238}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 2);
    CreateButton("SaveMenu_Slot3", {21, 258}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 3);
    CreateButton("SaveMenu_Slot4", {21, 278}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 4);
    CreateButton("SaveMenu_Slot5", {21, 298}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 5);
    CreateButton("SaveMenu_Slot6", {21, 318}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 6);

    pBtnLoadSlot = CreateButton("SaveMenu_Save", {241, 302}, {105, 40}, 1, 0, UIMSG_SaveLoadBtn, 0, Io::InputAction::Invalid, "", {saveload_ui_ls_saved});
    pBtnCancel = CreateButton({350, 302}, {105, 40}, 1, 0, UIMSG_Cancel, 0, Io::InputAction::Invalid, "", {saveload_ui_x_d});
    pBtnArrowUp = CreateButton({215, 199}, {17, 17}, 1, 0, UIMSG_ArrowUp, 0, Io::InputAction::Invalid, "", {ui_ar_up_dn});
    pBtnDownArrow = CreateButton({215, 323}, {17, 17}, 1, 0, UIMSG_DownArrow, MAX_SAVE_SLOTS, Io::InputAction::Invalid, "", {ui_ar_dn_dn});

    CreateButton({215, 216}, {17, 107}, 1, 0, UIMSG_SaveLoadScroll, MAX_SAVE_SLOTS);
}

void GUIWindow_Save::Update() {
    if (GetCurrentMenuID() != MENU_SAVELOAD && GetCurrentMenuID() != MENU_LoadingProcInMainMenu) {
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
        render->DrawTextureNew(241 / 640.0f, 302 / 480.0f, saveload_ui_saveu);
        render->DrawTextureNew(18 / 640.0f, 139 / 480.0f, saveload_ui_save_up);
        render->DrawTextureNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }
    UI_DrawSaveLoad(true);
}

GUIWindow_Load::GUIWindow_Load(bool ingame) : GUIWindow(WINDOW_Load, {0, 0}, {0, 0}) {
    current_screen_type = SCREEN_LOADGAME;

    saveload_ui_loadsave = assets->getImage_ColorKey("loadsave");
    saveload_ui_load_up = assets->getImage_ColorKey("load_up");
    saveload_ui_loadu = assets->getImage_ColorKey("LS_loadU");
    saveload_ui_x_u = assets->getImage_ColorKey("x_u");

    main_menu_background = nullptr;
    if (!ingame) {
        main_menu_background = assets->getImage_PCXFromIconsLOD("lsave640.pcx");
        render->DrawTextureNew(0, 0, main_menu_background);
    } else {
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
        render->DrawTextureNew(241 / 640.0f, 302 / 480.0f, saveload_ui_loadu);
        render->DrawTextureNew(18 / 640.0f, 139 / 480.0f, saveload_ui_load_up);
        render->DrawTextureNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }

    // GUIWindow::GUIWindow
    this->uFrameX = saveload_dlg_xs[ingame ? 1 : 0];
    this->uFrameY = saveload_dlg_ys[ingame ? 1 : 0];
    this->uFrameWidth = saveload_dlg_zs[ingame ? 1 : 0];
    this->uFrameHeight = saveload_dlg_ws[ingame ? 1 : 0];
    this->uFrameZ = uFrameX + uFrameWidth - 1;
    this->uFrameW = uFrameY + uFrameHeight - 1;

    DrawText(assets->pFontSmallnum.get(), {25, 199}, colorTable.White, localization->GetString(LSTR_READING));
    render->Present();

    pSavegameList->Initialize();

    LodReader pLODFile;
    for (int i = 0; i < pSavegameList->numSavegameFiles; ++i) {
        std::string str = fmt::format("saves/{}", pSavegameList->pFileList[i]);
        if (!ufs->exists(str)) {
            pSavegameList->pSavegameUsedSlots[i] = false;
            pSavegameList->pSavegameHeader[i].name = localization->GetString(LSTR_EMPTY_SAVE);
            continue;
        }

        if (pSavegameList->pFileList[i] == pSavegameList->lastLoadedSave) {
            pSavegameList->selectedSlot = i;
            pSavegameList->saveListPosition = i;
            if (pSavegameList->saveListPosition + 7 > pSavegameList->numSavegameFiles) {
                pSavegameList->saveListPosition = std::max(0, pSavegameList->numSavegameFiles - 7);
            }
        }

        pLODFile.open(ufs->read(str), LOD_ALLOW_DUPLICATES);
        deserialize(pLODFile.read("header.bin"), &pSavegameList->pSavegameHeader[i], tags::via<SaveGameHeader_MM7>);

        if (ascii::noCaseEquals(pSavegameList->pFileList[i], localization->GetString(LSTR_AUTOSAVE_MM7))) { // TODO(captainurist): #unicode might not be ascii
            pSavegameList->pSavegameHeader[i].name = localization->GetString(LSTR_AUTOSAVE);
        }

        if (pSavegameList->pSavegameHeader[i].name.empty()) {
            // blank so add something - suspect quicksaves
            std::string newname = pSavegameList->pFileList[i];
            std::string test = newname.substr(0, newname.size() - 4);
            pSavegameList->pSavegameHeader[i].name = test;
        }

        pSavegameList->pSavegameThumbnails[i] = GraphicsImage::Create(std::make_unique<PCX_LOD_Raw_Loader>(&pLODFile, "image.pcx"));

        if (pSavegameList->pSavegameThumbnails[i]->width() == 0) {
            pSavegameList->pSavegameThumbnails[i]->Release();
            pSavegameList->pSavegameThumbnails[i] = nullptr;
        }

        pSavegameList->pSavegameUsedSlots[i] = true;
        //if (pSavegameList->pSavegameThumbnails[i] != nullptr) {
        //    pSavegameUsedSlots[i] = 1;
        //} else {
        //    pSavegameUsedSlots[i] = 0;
        //    pSavegameList->pFileList[i].clear();
        //}
    }

    saveload_ui_x_d = assets->getImage_ColorKey("x_d");
    saveload_ui_ls_saved = assets->getImage_ColorKey("LS_loadD");
    ui_ar_up_dn = assets->getImage_ColorKey("AR_UP_DN");
    ui_ar_dn_dn = assets->getImage_ColorKey("AR_DN_DN");

    scrollstop = assets->getImage_ColorKey("con_x");

    CreateButton("LoadMenu_Slot0", {21, 198}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 0);
    CreateButton("LoadMenu_Slot1", {21, 219}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 1);
    CreateButton("LoadMenu_Slot2", {21, 240}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 2);
    CreateButton("LoadMenu_Slot3", {21, 261}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 3);
    CreateButton("LoadMenu_Slot4", {21, 282}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 4);
    CreateButton("LoadMenu_Slot5", {21, 303}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 5);
    CreateButton("LoadMenu_Slot6", {21, 324}, {191, 18}, 1, 0, UIMSG_SelectLoadSlot, 6);

    pBtnLoadSlot = CreateButton("LoadMenu_Load", {241, 302}, {105, 40}, 1, 0, UIMSG_SaveLoadBtn, 0, Io::InputAction::Invalid, "", {saveload_ui_ls_saved});
    pBtnCancel = CreateButton({350, 302}, {105, 40}, 1, 0, UIMSG_Cancel, 0, Io::InputAction::Invalid, "", {saveload_ui_x_d});
    pBtnArrowUp = CreateButton({215, 199}, {17, 17}, 1, 0, UIMSG_ArrowUp, 0, Io::InputAction::Invalid, "", {ui_ar_up_dn});
    pBtnDownArrow = CreateButton({215, 323}, {17, 17}, 1, 0, UIMSG_DownArrow, pSavegameList->numSavegameFiles, Io::InputAction::Invalid, "", {ui_ar_dn_dn});

    CreateButton("LoadMenu_Scroll", {215, 216}, {17, 107}, 1, 0, UIMSG_SaveLoadScroll, pSavegameList->numSavegameFiles);
}

void GUIWindow_Load::Update() {
    if (main_menu_background != nullptr) {
        render->DrawTextureNew(0, 0, main_menu_background);
    }
    if (GetCurrentMenuID() != MENU_SAVELOAD && GetCurrentMenuID() != MENU_LoadingProcInMainMenu) {
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, saveload_ui_loadsave);
        render->DrawTextureNew(241 / 640.0f, 302 / 480.0f, saveload_ui_loadu);
        render->DrawTextureNew(18 / 640.0f, 139 / 480.0f, saveload_ui_load_up);
        render->DrawTextureNew(351 / 640.0f, 302 / 480.0f, saveload_ui_x_u);
    }
    UI_DrawSaveLoad(false);
}

static void UI_DrawSaveLoad(bool save) {
    if (pSavegameList->pSavegameUsedSlots[pSavegameList->selectedSlot]) {
        GUIWindow save_load_window;
        save_load_window.uFrameX = pGUIWindow_CurrentMenu->uFrameX + 240;
        save_load_window.uFrameWidth = 220;
        save_load_window.uFrameY = (pGUIWindow_CurrentMenu->uFrameY - assets->pFontSmallnum->GetHeight()) + 157;
        save_load_window.uFrameZ = save_load_window.uFrameX + 219;
        save_load_window.uFrameHeight = assets->pFontSmallnum->GetHeight();
        save_load_window.uFrameW = assets->pFontSmallnum->GetHeight() + save_load_window.uFrameY - 1;
        if (pSavegameList->pSavegameThumbnails[pSavegameList->selectedSlot]) {
            render->DrawTextureNew((pGUIWindow_CurrentMenu->uFrameX + 276) / 640.0f, (pGUIWindow_CurrentMenu->uFrameY + 171) / 480.0f,
                                   pSavegameList->pSavegameThumbnails[pSavegameList->selectedSlot]);
        }
        // Draw map name
        save_load_window.DrawTitleText(assets->pFontSmallnum.get(), 0, 0, colorTable.White,
                                       pMapStats->pInfos[pMapStats->GetMapInfo(pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].locationName)].name, 3);

        // Draw date
        CivilTime time = pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].playingTime.toCivilTime();

        save_load_window.uFrameY = pGUIWindow_CurrentMenu->uFrameY + 261;

        std::string str = fmt::format(
            "{} {}:{:02} {}\n{} {} {}",
            localization->GetDayName(time.dayOfWeek - 1),
            time.hourAmPm,
            time.minute,
            localization->GetAmPm(time.isPm),
            time.day,
            localization->GetMonthName(time.month - 1),
            time.year);
        save_load_window.DrawTitleText(assets->pFontSmallnum.get(), 0, 0, colorTable.White, str, 3);
    }

    if (pGUIWindow_CurrentMenu->keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        pGUIWindow_CurrentMenu->keyboard_input_status = WINDOW_INPUT_NONE;
        pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].name = keyboardInputHandler->GetTextInput();
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_SaveGame, 0, 0);
    } else {
        if (pGUIWindow_CurrentMenu->keyboard_input_status == WINDOW_INPUT_CANCELLED)
            pGUIWindow_CurrentMenu->keyboard_input_status = WINDOW_INPUT_NONE;
    }

    if (GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
        pGUIWindow_CurrentMenu->DrawText(assets->pFontSmallnum.get(),
            {assets->pFontSmallnum->AlignText_Center(186, localization->GetString(LSTR_LOADING)) + 25, 220}, colorTable.White,
            localization->GetString(LSTR_LOADING));
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontSmallnum.get(),
                                               {assets->pFontSmallnum->AlignText_Center(186, pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].name) + 25, 262}, colorTable.White,
                                               pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].name, 185, 0);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontSmallnum.get(),
            {assets->pFontSmallnum->AlignText_Center(186, localization->GetString(LSTR_PLEASE_WAIT)) + 25, 304}, colorTable.White,
            localization->GetString(LSTR_PLEASE_WAIT));
    } else {
        int maxSaveFiles = MAX_SAVE_SLOTS;
        int framex = 0, framey = 0;
        int stopPos = 0;

        if (!save) {
            maxSaveFiles = pSavegameList->numSavegameFiles;
            framex = pGUIWindow_CurrentMenu->uFrameX;
            framey = pGUIWindow_CurrentMenu->uFrameY;
        }

        if (maxSaveFiles > 7) {
            stopPos = (float(pSavegameList->saveListPosition) / (maxSaveFiles - 7)) * 89.0f;
        }
        if (pSavegameList->saveListPosition > maxSaveFiles - 7) {
            stopPos = 89;
        }
        render->DrawTextureNew((216 + framex) / 640.f, (217 + framey + stopPos) / 480.f, scrollstop);

        int slot_Y = 199;
        for (int i = pSavegameList->saveListPosition; i < maxSaveFiles; ++i) {
            if (slot_Y >= 346) {
                break;
            }
            if (pGUIWindow_CurrentMenu->keyboard_input_status != WINDOW_INPUT_IN_PROGRESS || i != pSavegameList->selectedSlot) {
                pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontSmallnum.get(), {27, slot_Y}, i == pSavegameList->selectedSlot ? colorTable.LaserLemon : colorTable.White,
                                                       pSavegameList->pSavegameHeader[i].name, 185, 0);
            } else {
                pGUIWindow_CurrentMenu->DrawFlashingInputCursor(pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontSmallnum.get(), {27, slot_Y},
                    i == pSavegameList->selectedSlot ? colorTable.LaserLemon : colorTable.White, keyboardInputHandler->GetTextInput(), 175, 1) + 27, slot_Y, assets->pFontSmallnum.get());
            }
            slot_Y += 21;
        }
    }
}

void GUIWindow_Load::slotSelected(int slotIndex) {
    // main menu save/load wnd   clicking on savegame lines
    if (pGUIWindow_CurrentMenu->keyboard_input_status == WINDOW_INPUT_IN_PROGRESS)
        keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_NONE);
    assert(current_screen_type != SCREEN_SAVEGAME); // No savegame in main menu
    if (isLoadSlotClicked && pSavegameList->selectedSlot == slotIndex + pSavegameList->saveListPosition) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_LoadGame, 0, 0);
    } else {
        pSavegameList->selectedSlot = slotIndex + pSavegameList->saveListPosition;
        isLoadSlotClicked = true;
    }
}

void GUIWindow_Load::loadButtonPressed() {
    new OnSaveLoad({ pGUIWindow_CurrentMenu->uFrameX + 241, pGUIWindow_CurrentMenu->uFrameY + 302 }, { 61, 28 }, pBtnLoadSlot);
}

void GUIWindow_Load::downArrowPressed(int maxSlots) {
    if (pSavegameList->saveListPosition + 7 < maxSlots) {
        ++pSavegameList->saveListPosition;
    }
    new OnButtonClick2({ pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 323 }, { 0, 0 }, pBtnDownArrow);
}

void GUIWindow_Load::upArrowPressed() {
    --pSavegameList->saveListPosition;
    if (pSavegameList->saveListPosition < 0)
        pSavegameList->saveListPosition = 0;
    new OnButtonClick2({ pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 197 }, { 0, 0 }, pBtnArrowUp);
}

void GUIWindow_Load::cancelButtonPressed() {
    new OnCancel3({ pGUIWindow_CurrentMenu->uFrameX + 350, pGUIWindow_CurrentMenu->uFrameY + 302 }, { 61, 28 }, pBtnCancel);
}

void GUIWindow_Load::scroll(int maxSlots) {
    // pskelton add for scroll click
    if (maxSlots < 7) {
        // Too few saves to scroll yet
        return;
    }
    Pointi mousePos = mouse->position();
    int mx = mousePos.x, my = mousePos.y;
    // 276 is offset down from top (216 + 60 frame)
    my -= 276;
    // 107 is total height of bar
    float fmy = static_cast<float>(my) / 107.0f;
    int newlistpost = std::round((maxSlots - 7) * fmy);
    newlistpost = std::clamp(newlistpost, 0, (maxSlots - 7));
    pSavegameList->saveListPosition = newlistpost;
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
}

void GUIWindow_Load::quickLoad() {
    int slot = GetQuickSaveSlot();
    if (slot != -1) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
        pSavegameList->selectedSlot = slot;
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_LoadGame, 0, 0);
    } else {
        logger->error("QuickLoadGame:: No quick save could be found!");
        pAudioPlayer->playUISound(SOUND_error);
    }
}
