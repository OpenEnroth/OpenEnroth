#include "GUI/UI/UISaveLoad.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Tables/MapTable.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Snapshots/CompositeSnapshots.h"

#include "Media/Audio/AudioPlayer.h"

#include "Io/KeyboardInputHandler.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"

#include "Library/Image/Pcx.h"
#include "Library/Logger/Logger.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/String/Ascii.h"
#include "Utility/Exception.h"

using Io::TextInputType;

std::array<int, 2> saveload_dlg_xs = {{82, 0}};
std::array<int, 2> saveload_dlg_ys = {{60, 0}};
std::array<int, 2> saveload_dlg_zs = {{460, 640}};
std::array<int, 2> saveload_dlg_ws = {{344, 480}};

GraphicsImage *saveload_ui_ls_saved = nullptr;
GraphicsImage *saveload_ui_x_d = nullptr;

static GraphicsImage *scrollstop = nullptr;

/**
 * @return                          Slots to show in the load menu - all saves from the saves folder, sorted by
 *                                  display name with file name as a tie-breaker.
 */
static std::vector<SavegameSlot> loadMenuSlots() {
    std::vector<SavegameSlot> result;

    if (!ufs->exists("saves"))
        return result;

    for (const auto &entry : ufs->ls("saves")) {
        if (entry.type != FILE_REGULAR || !entry.name.ends_with(".mm7"))
            continue;

        SavegameSlot slot;
        slot.fileName = entry.name;

        SaveGameLite save;
        try {
            deserialize(ufs->read(fmt::format("saves/{}", entry.name)), &save, tags::via<SaveGameLite_MM7>);
        } catch (const std::exception &e) {
            MM_WARNING("Couldn't load savegame '{}': {}", entry.name, e.what()); // Don't list unreadable saves.
            continue;
        }
        slot.header = save.header;

        // Show autosave & quicksave titles in the current language - the stored titles are in whatever language
        // was active when the save was made.
        if (ascii::noCaseEquals(slot.fileName, autosaveFileName))
            slot.header.name = localization->str(LSTR_AUTOSAVE);
        else if (ascii::noCaseStartsWith(slot.fileName, quickSaveFileNamePrefix))
            slot.header.name = localization->str(LSTR_QUICKSAVE);

        if (slot.header.name.empty()) // Foreign saves with blank titles - fall back to the file name.
            slot.header.name = slot.fileName.substr(0, slot.fileName.size() - 4);

        try {
            slot.thumbnail.reset(GraphicsImage::Create(pcx::decode(save.thumbnail))); // TODO(captainurist): lazy-load.

            if (slot.thumbnail->width() == 0)
                slot.thumbnail = nullptr;
        } catch (const Exception &e) {
            MM_DEBUG("Savegame thumbnail exception: {}", e.what()); // swallow it - bad pcx thumbnail is fine
            slot.thumbnail = nullptr;
        }

        result.push_back(std::move(slot));
    }

    std::ranges::sort(result, [](const SavegameSlot &l, const SavegameSlot &r) {
        if (int result = ascii::noCaseCompare(l.header.name, r.header.name))
            return result < 0;
        return l.fileName < r.fileName;
    });
    return result;
}

/**
 * @return                          Slots to show in the save menu - a new save slot first, then the saves. Autosave
 *                                  & quicksaves are loadable, but shouldn't be manually saved over, so they are
 *                                  not shown.
 */
static std::vector<SavegameSlot> saveMenuSlots() {
    std::vector<SavegameSlot> result = loadMenuSlots();
    std::erase_if(result, [](const SavegameSlot &slot) {
        return ascii::noCaseEquals(slot.fileName, autosaveFileName) ||
               ascii::noCaseStartsWith(slot.fileName, quickSaveFileNamePrefix);
    });
    result.insert(result.begin(), SavegameSlot()); // New save slot goes first.
    return result;
}

GUIWindow_SaveLoad *saveLoadMenu() {
    assert(current_screen_type == SCREEN_SAVEGAME || current_screen_type == SCREEN_LOADGAME);
    return static_cast<GUIWindow_SaveLoad *>(pGUIWindow_CurrentMenu.get());
}

GUIWindow_SaveLoad::GUIWindow_SaveLoad(WindowType type, Pointi position, Sizei dimensions)
    : GUIWindow(type, position, dimensions)
{}

GUIWindow_SaveLoad::~GUIWindow_SaveLoad() {
    keyboardInputHandler->EndTextInput(this); // Save slots are renamed in place.
}

void GUIWindow_SaveLoad::preselectSlot(std::string_view fileName) {
    auto pos = std::ranges::find(_slots, fileName, &SavegameSlot::fileName);
    if (pos == _slots.end())
        return;
    _selectedSlot = pos - _slots.begin();
    _scrollPosition = std::min(_selectedSlot, std::max<int>(0, std::ssize(_slots) - 7));
}

void GUIWindow_SaveLoad::scrollUp() {
    _scrollPosition = std::max(0, _scrollPosition - 1);
}

void GUIWindow_SaveLoad::scrollDown() {
    if (_scrollPosition + 7 < std::ssize(_slots))
        ++_scrollPosition;
}

void GUIWindow_SaveLoad::scrollWithMouse(Pointi mousePos) {
    int slotCount = std::ssize(_slots);
    if (slotCount < 7)
        return; // Too few saves to scroll yet.
    // 216 is the scroll bar offset from the top of the dialog, 107 is its total height.
    int my = mousePos.y - frameRect.y - 216;
    float fmy = static_cast<float>(my) / 107.0f;
    int newPosition = std::round((slotCount - 7) * fmy);
    _scrollPosition = std::clamp(newPosition, 0, slotCount - 7);
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
}

void GUIWindow_SaveLoad::drawSaveLoad() {
    if (hasSelectedSlot() && !selectedSlot().fileName.empty()) {
        const SavegameSlot &slot = selectedSlot();
        Recti titleRect(frameRect.x + 240, (frameRect.y - assets->pFontSmallnum->GetHeight()) + 157,
                        220, assets->pFontSmallnum->GetHeight());
        if (slot.thumbnail)
            render->DrawQuad2D(slot.thumbnail.get(), {frameRect.x + 276, frameRect.y + 171});

        // Draw map name
        GUIWindow::DrawTitleText(assets->pFontSmallnum.get(), 0, 0, colorTable.White,
                                 pMapTable->pInfos[pMapTable->GetMapInfo(slot.header.locationName)].name, 3, titleRect);

        // Draw date
        CivilTime time = slot.header.playingTime.toCivilTime();
        titleRect.y = frameRect.y + 261;
        std::string str = fmt::format(
            "{} {}:{:02} {}\n{} {} {}",
            localization->dayName(time.dayOfWeek - 1),
            time.hourAmPm,
            time.minute,
            localization->amPm(time.isPm),
            time.day,
            localization->monthName(time.month - 1),
            time.year);
        GUIWindow::DrawTitleText(assets->pFontSmallnum.get(), 0, 0, colorTable.White, str, 3, titleRect);
    }

    if (keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        keyboard_input_status = WINDOW_INPUT_NONE;
        if (keyboardInputHandler->GetTextInput().empty()) {
            // Saves need a name, keep the input open.
            keyboardInputHandler->StartTextInput(TextInputType::Text, 19, this);
        } else {
            setSelectedSlotName(keyboardInputHandler->GetTextInput());
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_SaveGame, 0, 0);
        }
    }

    if (GetCurrentMenuID() == MENU_LoadingProcInMainMenu) {
        const std::string &name = selectedSlot().header.name;
        GUIWindow::DrawText(assets->pFontSmallnum.get(),
            {assets->pFontSmallnum->AlignText_Center(186, localization->str(LSTR_LOADING)) + 25, 220}, colorTable.White,
            localization->str(LSTR_LOADING), frameRect);
        DrawTextInRect(assets->pFontSmallnum.get(),
                       {assets->pFontSmallnum->AlignText_Center(186, name) + 25, 262}, colorTable.White, name, 185, 0);
        GUIWindow::DrawText(assets->pFontSmallnum.get(),
            {assets->pFontSmallnum->AlignText_Center(186, localization->str(LSTR_PLEASE_WAIT)) + 25, 304}, colorTable.White,
            localization->str(LSTR_PLEASE_WAIT), frameRect);
    } else {
        int slotCount = std::ssize(_slots);
        int stopPos = 0;
        if (slotCount > 7) {
            stopPos = (float(_scrollPosition) / (slotCount - 7)) * 89.0f;
            if (_scrollPosition > slotCount - 7)
                stopPos = 89;
        }
        render->DrawQuad2D(scrollstop, {216 + frameRect.x, static_cast<int>(217 + frameRect.y + stopPos)});

        int slot_Y = 199;
        for (int i = _scrollPosition; i < slotCount; ++i) {
            if (slot_Y >= 346)
                break;
            bool isNewSaveSlot = _slots[i].fileName.empty();
            if (keyboard_input_status != WINDOW_INPUT_IN_PROGRESS || i != _selectedSlot) {
                DrawTextInRect(assets->pFontSmallnum.get(), {27, slot_Y}, i == _selectedSlot ? colorTable.LaserLemon : colorTable.White,
                               isNewSaveSlot ? localization->str(LSTR_NEW_SAVE) : _slots[i].header.name, 185, 0);
            } else {
                GUIWindow::DrawFlashingInputCursor(DrawTextInRect(assets->pFontSmallnum.get(), {27, slot_Y},
                    colorTable.LaserLemon, keyboardInputHandler->GetTextInput(), 175, 1) + 27, slot_Y, assets->pFontSmallnum.get(), frameRect);
            }
            slot_Y += 21;
        }
    }
}

GUIWindow_Save::GUIWindow_Save() : GUIWindow_SaveLoad(WINDOW_Save, {0, 0}, render->GetRenderDimensions()) {
    saveload_ui_loadsave = assets->getImage_ColorKey("loadsave");
    saveload_ui_save_up = assets->getImage_ColorKey("save_up");
    saveload_ui_saveu = assets->getImage_ColorKey("LS_saveU");
    saveload_ui_x_u = assets->getImage_ColorKey("x_u");

    _slots = saveMenuSlots(); // New save slot is shown first.

    // Pre-select the currently loaded save, so that just clicking the save button saves over it. There might be
    // none, e.g. when playing a new game - then the new save slot stays selected.
    preselectSlot(engine->_lastLoadedSaveFileName);

    saveload_ui_x_d = assets->getImage_ColorKey("x_d");
    saveload_ui_ls_saved = assets->getImage_ColorKey("LS_saveD");
    ui_ar_up_dn = assets->getImage_ColorKey("ar_up_dn");
    ui_ar_dn_dn = assets->getImage_ColorKey("ar_dn_dn");

    scrollstop = assets->getImage_ColorKey("con_x");

    CreateButton("SaveMenu_Slot0", {21, 198}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 0);
    CreateButton("SaveMenu_Slot1", {21, 218}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 1);
    CreateButton("SaveMenu_Slot2", {21, 238}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 2);
    CreateButton("SaveMenu_Slot3", {21, 258}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 3);
    CreateButton("SaveMenu_Slot4", {21, 278}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 4);
    CreateButton("SaveMenu_Slot5", {21, 298}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 5);
    CreateButton("SaveMenu_Slot6", {21, 318}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 6);

    pBtnLoadSlot = CreateButton("SaveMenu_Save", {241, 302}, {105, 40}, BUTTON_TYPE_NORMAL, 0, UIMSG_SaveLoadBtn, 0, INPUT_ACTION_INVALID, "", {saveload_ui_ls_saved});
    pBtnCancel = CreateButton({350, 302}, {105, 40}, BUTTON_TYPE_NORMAL, 0, UIMSG_Cancel, 0, INPUT_ACTION_INVALID, "", {saveload_ui_x_d});
    pBtnArrowUp = CreateButton({215, 199}, {17, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_ArrowUp, 0, INPUT_ACTION_INVALID, "", {ui_ar_up_dn});
    pBtnDownArrow = CreateButton({215, 323}, {17, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_DownArrow, 0, INPUT_ACTION_INVALID, "", {ui_ar_dn_dn});

    CreateButton({215, 216}, {17, 107}, BUTTON_TYPE_NORMAL, 0, UIMSG_SaveLoadScroll, 0);
}

void GUIWindow_Save::Update() {
    if (GetCurrentMenuID() != MENU_SAVELOAD && GetCurrentMenuID() != MENU_LoadingProcInMainMenu) {
        render->DrawQuad2D(saveload_ui_loadsave, {8, 8});
        render->DrawQuad2D(saveload_ui_saveu, {241, 302});
        render->DrawQuad2D(saveload_ui_save_up, {18, 139});
        render->DrawQuad2D(saveload_ui_x_u, {351, 302});
    }
    drawSaveLoad();
}

void GUIWindow_Save::slotClicked(int slotIndex, bool isDoubleClick) {
    if (keyboard_input_status == WINDOW_INPUT_IN_PROGRESS)
        keyboardInputHandler->EndTextInput();
    int slot = _scrollPosition + slotIndex;
    if (slot >= std::ssize(_slots))
        return; // Clicked below the last slot.
    _selectedSlot = slot;
    if (isDoubleClick) {
        keyboardInputHandler->StartTextInput(TextInputType::Text, 19, this);
        if (!selectedSlot().fileName.empty())
            keyboardInputHandler->SetTextInput(selectedSlot().header.name);
    }
}

GUIWindow_Load::GUIWindow_Load(bool ingame) : GUIWindow_SaveLoad(WINDOW_Load, {0, 0}, {0, 0}) {
    current_screen_type = SCREEN_LOADGAME;

    saveload_ui_loadsave = assets->getImage_ColorKey("loadsave");
    saveload_ui_load_up = assets->getImage_ColorKey("load_up");
    saveload_ui_loadu = assets->getImage_ColorKey("LS_loadU");
    saveload_ui_x_u = assets->getImage_ColorKey("x_u");

    main_menu_background = nullptr;
    if (!ingame) {
        main_menu_background = assets->getImage_PCXFromIconsLOD("lsave640.pcx");
        render->DrawQuad2D(main_menu_background, {0, 0});
    } else {
        render->DrawQuad2D(saveload_ui_loadsave, {8, 8});
        render->DrawQuad2D(saveload_ui_loadu, {241, 302});
        render->DrawQuad2D(saveload_ui_load_up, {18, 139});
        render->DrawQuad2D(saveload_ui_x_u, {351, 302});
    }

    this->frameRect = Recti(saveload_dlg_xs[ingame ? 1 : 0], saveload_dlg_ys[ingame ? 1 : 0],
                            saveload_dlg_zs[ingame ? 1 : 0], saveload_dlg_ws[ingame ? 1 : 0]);

    DrawText(assets->pFontSmallnum.get(), {25, 199}, colorTable.White, localization->str(LSTR_READING), this->frameRect);
    render->Present();

    _slots = loadMenuSlots();

    // Pre-select the last loaded save.
    preselectSlot(engine->_lastLoadedSaveFileName);

    saveload_ui_x_d = assets->getImage_ColorKey("x_d");
    saveload_ui_ls_saved = assets->getImage_ColorKey("LS_loadD");
    ui_ar_up_dn = assets->getImage_ColorKey("AR_UP_DN");
    ui_ar_dn_dn = assets->getImage_ColorKey("AR_DN_DN");

    scrollstop = assets->getImage_ColorKey("con_x");

    CreateButton("LoadMenu_Slot0", {21, 198}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 0);
    CreateButton("LoadMenu_Slot1", {21, 219}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 1);
    CreateButton("LoadMenu_Slot2", {21, 240}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 2);
    CreateButton("LoadMenu_Slot3", {21, 261}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 3);
    CreateButton("LoadMenu_Slot4", {21, 282}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 4);
    CreateButton("LoadMenu_Slot5", {21, 303}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 5);
    CreateButton("LoadMenu_Slot6", {21, 324}, {191, 18}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectLoadSlot, 6);

    pBtnLoadSlot = CreateButton("LoadMenu_Load", {241, 302}, {105, 40}, BUTTON_TYPE_NORMAL, 0, UIMSG_SaveLoadBtn, 0, INPUT_ACTION_INVALID, "", {saveload_ui_ls_saved});
    pBtnCancel = CreateButton({350, 302}, {105, 40}, BUTTON_TYPE_NORMAL, 0, UIMSG_Cancel, 0, INPUT_ACTION_INVALID, "", {saveload_ui_x_d});
    pBtnArrowUp = CreateButton({215, 199}, {17, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_ArrowUp, 0, INPUT_ACTION_INVALID, "", {ui_ar_up_dn});
    pBtnDownArrow = CreateButton({215, 323}, {17, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_DownArrow, 0, INPUT_ACTION_INVALID, "", {ui_ar_dn_dn});

    CreateButton("LoadMenu_Scroll", {215, 216}, {17, 107}, BUTTON_TYPE_NORMAL, 0, UIMSG_SaveLoadScroll, 0);
}

void GUIWindow_Load::Update() {
    if (main_menu_background != nullptr) {
        render->DrawQuad2D(main_menu_background, {0, 0});
    }
    if (GetCurrentMenuID() != MENU_SAVELOAD && GetCurrentMenuID() != MENU_LoadingProcInMainMenu) {
        render->DrawQuad2D(saveload_ui_loadsave, {8, 8});
        render->DrawQuad2D(saveload_ui_loadu, {241, 302});
        render->DrawQuad2D(saveload_ui_load_up, {18, 139});
        render->DrawQuad2D(saveload_ui_x_u, {351, 302});
    }
    drawSaveLoad();
}

void GUIWindow_Load::slotClicked(int slotIndex, bool isDoubleClick) {
    if (keyboard_input_status == WINDOW_INPUT_IN_PROGRESS)
        keyboardInputHandler->EndTextInput();
    int slot = _scrollPosition + slotIndex;
    if (slot >= std::ssize(_slots))
        return; // Clicked below the last save.
    _selectedSlot = slot;
    if (isDoubleClick)
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_LoadGame, 0, 0);
}

void GUIWindow_Load::loadButtonPressed() {
    new OnSaveLoad({ frameRect.x + 241, frameRect.y + 302 }, { 61, 28 }, pBtnLoadSlot);
}

void GUIWindow_Load::downArrowPressed() {
    scrollDown();
    new OnButtonClick({ frameRect.x + 215, frameRect.y + 323 }, { 0, 0 }, pBtnDownArrow);
}

void GUIWindow_Load::upArrowPressed() {
    scrollUp();
    new OnButtonClick({ frameRect.x + 215, frameRect.y + 197 }, { 0, 0 }, pBtnArrowUp);
}

void GUIWindow_Load::cancelButtonPressed() {
    new OnCancel3({ frameRect.x + 350, frameRect.y + 302 }, { 61, 28 }, pBtnCancel);
}

void GUIWindow_Load::quickLoad() {
    auto pos = std::ranges::find(_slots, getCurrentQuickSave(), &SavegameSlot::fileName);
    if (pos != _slots.end()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
        _selectedSlot = pos - _slots.begin();
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_LoadGame, 0, 0);
    } else {
        MM_ERROR("QuickLoadGame:: No quick save could be found!");
        pAudioPlayer->playUISound(SOUND_error);
    }
}
