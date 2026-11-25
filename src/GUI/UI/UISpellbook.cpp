#include "GUI/UI/UISpellbook.h"

#include <memory>
#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/GameType.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Random/Random.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Spells/SpellEnumFunctions.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Timer.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

static constexpr IndexedArray<const char *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> spellbook_texture_filename_suffices = {
    {MAGIC_SCHOOL_FIRE,   "f"},
    {MAGIC_SCHOOL_AIR,    "a"},
    {MAGIC_SCHOOL_WATER,  "w"},
    {MAGIC_SCHOOL_EARTH,  "e"},
    {MAGIC_SCHOOL_SPIRIT, "s"},
    {MAGIC_SCHOOL_MIND,   "m"},
    {MAGIC_SCHOOL_BODY,   "b"},
    {MAGIC_SCHOOL_LIGHT,  "l"},
    {MAGIC_SCHOOL_DARK,   "d"}
};

static constexpr IndexedArray<std::array<unsigned char, 12>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> pSpellbookSpellIndices = {{
    {MAGIC_SCHOOL_FIRE,     {0, 3, 1, 8, 11, 7, 4, 10, 6, 2, 5, 9}},
    {MAGIC_SCHOOL_AIR,      {0, 11, 2, 9, 6, 8, 5, 10, 3, 7, 1, 4}},
    {MAGIC_SCHOOL_WATER,    {0, 4, 8, 9, 1, 10, 3, 11, 7, 6, 2, 5}},
    {MAGIC_SCHOOL_EARTH,    {0, 7, 10, 8, 2, 11, 1, 5, 3, 6, 4, 9}},
    {MAGIC_SCHOOL_SPIRIT,   {0, 5, 10, 11, 7, 2, 8, 1, 4, 9, 3, 6}},
    {MAGIC_SCHOOL_MIND,     {0, 5, 9, 8, 3, 7, 6, 4, 1, 11, 2, 10}},
    {MAGIC_SCHOOL_BODY,     {0, 1, 6, 9, 3, 5, 8, 11, 7, 10, 4, 2}},
    {MAGIC_SCHOOL_LIGHT,    {0, 1, 10, 11, 9, 4, 3, 6, 5, 7, 8, 2}},
    {MAGIC_SCHOOL_DARK,     {0, 9, 3, 7, 1, 5, 2, 10, 11, 8, 6, 4}}
}};

static constexpr IndexedArray<const char *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> texNames = {
    {MAGIC_SCHOOL_FIRE,   "SBFB00"},
    {MAGIC_SCHOOL_AIR,    "SBAB00"},
    {MAGIC_SCHOOL_WATER,  "SBWB00"},
    {MAGIC_SCHOOL_EARTH,  "SBEB00"},
    {MAGIC_SCHOOL_SPIRIT, "SBSB00"},
    {MAGIC_SCHOOL_MIND,   "SBMB00"},
    {MAGIC_SCHOOL_BODY,   "SBBB00"},
    {MAGIC_SCHOOL_LIGHT,  "SBLB00"},
    {MAGIC_SCHOOL_DARK,   "SBDB00"}
};

// MM8's own school-tab column (see initializeTextures()) has no MM7-style "flip left when
// selected" shift - the "d"(im)/"u"(p) art itself already carries the selected/unselected look,
// so both states share one position per school.
static constexpr IndexedArray<std::array<int, 2>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> texture_tab_coord1 = {{
    {MAGIC_SCHOOL_FIRE,     {406, 9}},
    {MAGIC_SCHOOL_AIR,      {406, 46}},
    {MAGIC_SCHOOL_WATER,    {406, 84}},
    {MAGIC_SCHOOL_EARTH,    {406, 121}},
    {MAGIC_SCHOOL_SPIRIT,   {407, 158}},
    {MAGIC_SCHOOL_MIND,     {405, 196}},
    {MAGIC_SCHOOL_BODY,     {405, 234}},
    {MAGIC_SCHOOL_LIGHT,    {405, 272}},
    {MAGIC_SCHOOL_DARK,     {405, 309}}
}};

static constexpr IndexedArray<std::array<int, 2>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> texture_tab_coord0 = {{
    {MAGIC_SCHOOL_FIRE,     {415, 10}},
    {MAGIC_SCHOOL_AIR,      {415, 46}},
    {MAGIC_SCHOOL_WATER,    {415, 83}},
    {MAGIC_SCHOOL_EARTH,    {415, 121}},
    {MAGIC_SCHOOL_SPIRIT,   {415, 158}},
    {MAGIC_SCHOOL_MIND,     {416, 196}},
    {MAGIC_SCHOOL_BODY,     {416, 234}},
    {MAGIC_SCHOOL_LIGHT,    {416, 271}},
    {MAGIC_SCHOOL_DARK,     {416, 307}}
}};

// MM8's school-tab column runs down the far left edge of the frame, not MM7's
// mid-book x~399-416 seam. One fixed slot per school, 32px pitch, matching the "st{suffix}d/u"
// medallions' own 48x39 native size.
static constexpr Pointi kMm8SpellbookTabOrigin = {0, 20};
static constexpr int kMm8SpellbookTabPitch = 32;
static constexpr Sizei kMm8SpellbookTabSize = {48, 39};

// MM8's action-icon column, immediately below the tabs (three stacked buttons, flush left edge, no gap):
//   {0,320} 42x43 "gf-aspell"/"gf-aspelld" - Quick Spell. Not drawn: the entries are in "00 add.icons.lod",
//                                            which the engine does not open. kMm8QuickSpellSize is the hitbox.
//   {0,363} 37x46 "stssu"/"stssd" (icons.lod) - role unknown, decorative only (see initializeTextures())
//   {0,409} 37x44 "stxtu"/"stxtd" (icons.lod) - Close
static constexpr Pointi kMm8QuickSpellPos = {0, 320};
static constexpr Sizei kMm8QuickSpellSize = {42, 43};
static constexpr Pointi kMm8ClosePos = {0, 409};

SpellId spellbookSelectedSpell;

GUIWindow_Spellbook::GUIWindow_Spellbook() : GUIWindow(WINDOW_SPELL_BOOK, {0, 0}, render->GetRenderDimensions()) {
    current_screen_type = SCREEN_SPELL_BOOK;
    gameTimer->setPaused(true);

    initializeTextures();
    openSpellbook();

    // Sound 48 is absent in MM7
    pAudioPlayer->playUISound(SOUND_48);
}

void GUIWindow_Spellbook::openSpellbookPage(MagicSchool page) {
    onCloseSpellBookPage();
    pParty->activeCharacter().lastOpenedSpellbookPage = page;
    openSpellbook();
    pAudioPlayer->playUISound(vrng->randomBool() ? SOUND_TurnPage2 : SOUND_TurnPage1);
}

void GUIWindow_Spellbook::openSpellbook() {
    int pageSpells = 0;
    const Character &player = pParty->activeCharacter();

    loadSpellbook();

    MagicSchool chapter = player.lastOpenedSpellbookPage;
    for (SpellId spell : spellsForMagicSchool(chapter)) {
        if (!player.bHaveSpell[spell] && !engine->config->debug.AllMagic.value())
            continue;

        int index = spellIndexInMagicSchool(spell);
        CreateButton(fmt::format("SpellBook_Spell{}", index),
                     {pViewport.x + pIconPos[chapter][pSpellbookSpellIndices[chapter][index + 1]].Xpos,
                     pViewport.y + pIconPos[chapter][pSpellbookSpellIndices[chapter][index + 1]].Ypos},
                     SBPageSSpellsTextureList[index + 1]->size(), BUTTON_TYPE_NORMAL, UIMSG_Spellbook_ShowHightlightedSpellInfo,
                     UIMSG_SelectSpell, std::to_underlying(spell));
        pageSpells++;
    }

    CreateButton({0, 0}, {0, 0}, BUTTON_TYPE_NORMAL, 0, UIMSG_SpellBook_PressTab, 0, INPUT_ACTION_NEXT_CHAR);
    if (pageSpells)
        setKeyboardControlGroup(pageSpells, true, 0, 0);

    static constexpr IndexedArray<Pointi, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> buttonPositions = {
        {MAGIC_SCHOOL_FIRE,     {399, 10}},
        {MAGIC_SCHOOL_AIR,      {399, 46}},
        {MAGIC_SCHOOL_WATER,    {399, 83}},
        {MAGIC_SCHOOL_EARTH,    {399, 121}},
        {MAGIC_SCHOOL_SPIRIT,   {399, 158}},
        {MAGIC_SCHOOL_MIND,     {400, 196}},
        {MAGIC_SCHOOL_BODY,     {400, 234}},
        {MAGIC_SCHOOL_LIGHT,    {400, 271}},
        {MAGIC_SCHOOL_DARK,     {400, 307}},
    };

    bool isMm8 = gameType() == GAME_TYPE_MM8;
    for (MagicSchool school : allMagicSchools())
        if (player.pActiveSkills[skillForMagicSchool(school)] || engine->config->debug.AllMagic.value()) {
            Pointi pos = isMm8 ? Pointi(kMm8SpellbookTabOrigin.x, kMm8SpellbookTabOrigin.y + kMm8SpellbookTabPitch * std::to_underlying(school))
                                : buttonPositions[school];
            Sizei size = isMm8 ? kMm8SpellbookTabSize : Sizei(50, 36);
            CreateButton(fmt::format("SpellBook_School{}", std::to_underlying(school)), pos,
                         size, BUTTON_TYPE_NORMAL, 0, UIMSG_OpenSpellbookPage, std::to_underlying(school), INPUT_ACTION_INVALID,
                         localization->spellSchoolName(school));
        }

    Pointi quickSpellPos = isMm8 ? kMm8QuickSpellPos : Pointi(476, 450);
    Pointi closePos = isMm8 ? kMm8ClosePos : Pointi(561, 450);
    Sizei quickSpellSize = ui_spellbook_btn_quckspell ? ui_spellbook_btn_quckspell->size() : kMm8QuickSpellSize;
    pBtn_InstallRemoveSpell = CreateButton(quickSpellPos, quickSpellSize, BUTTON_TYPE_NORMAL, UIMSG_HintSelectRemoveQuickSpellBtn,
                                           UIMSG_ClickInstallRemoveQuickSpellBtn, 0, INPUT_ACTION_INVALID, "", {ui_spellbook_btn_quckspell_click});
    pBtn_CloseBook = CreateButton(closePos, ui_spellbook_btn_close->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID,
                                  localization->str(LSTR_EXIT_DIALOGUE), {ui_spellbook_btn_close_click});
}

void GUIWindow_Spellbook::Update() {
    const Character &player = pParty->activeCharacter();
    int pX_coord, pY_coord;

    drawCurrentSchoolBackground();

    for (MagicSchool page : allMagicSchools()) {
        Skill skill = skillForMagicSchool(page);

        if (player.pActiveSkills[skill] || engine->config->debug.AllMagic.value()) {
            std::shared_ptr<GraphicsImage> pPageTexture;
            if (gameType() == GAME_TYPE_MM8) {
                // MM8's tab art is "st{suffix}d"/"st{suffix}u" (icons.lod), one fixed
                // left-edge slot per school (kMm8SpellbookTabOrigin/Pitch). Unlike MM7's mid-book
                // tabs (which draw a "selected" variant for the current page), the original shows no tab
                // for the open page. The button stays, clicking the current
                // page's own tab is a no-op in Game.cpp's UIMSG_OpenSpellbookPage.
                pPageTexture = player.lastOpenedSpellbookPage == page ? nullptr : ui_spellbook_school_tabs[page][0];
                pX_coord = kMm8SpellbookTabOrigin.x;
                pY_coord = kMm8SpellbookTabOrigin.y + kMm8SpellbookTabPitch * std::to_underlying(page);
            } else if (player.lastOpenedSpellbookPage == page) {
                pPageTexture = ui_spellbook_school_tabs[page][1];
                pX_coord = texture_tab_coord1[page][0];
                pY_coord = texture_tab_coord1[page][1];
            } else {
                pPageTexture = ui_spellbook_school_tabs[page][0];
                pX_coord = texture_tab_coord0[page][0];
                pY_coord = texture_tab_coord0[page][1];
            }
            render->DrawQuad2D(pPageTexture, {pX_coord, pY_coord});

            Pointi mousePos = mouse->position();

            for (SpellId spell : spellsForMagicSchool(player.lastOpenedSpellbookPage)) {
                int index = spellIndexInMagicSchool(spell);
                if (player.bHaveSpell[spell] || engine->config->debug.AllMagic.value()) {
                    // this should check if player knows spell
                    if (SBPageSSpellsTextureList[index + 1]) {
                        std::shared_ptr<GraphicsImage> pTexture = spellbookSelectedSpell == spell ? SBPageCSpellsTextureList[index + 1] : SBPageSSpellsTextureList[index + 1];
                        if (pTexture) {
                            SpellBookIconPos &iconPos = pIconPos[player.lastOpenedSpellbookPage][pSpellbookSpellIndices[player.lastOpenedSpellbookPage][index + 1]];

                            pX_coord = pViewport.x + iconPos.Xpos;
                            pY_coord = pViewport.y + iconPos.Ypos;

                            Recti iconRect = Recti(pX_coord, pY_coord, pTexture->width(), pTexture->height());
                            if (iconRect.contains(mousePos)) { // mouseover highlight
                                if (SBPageCSpellsTextureList[index + 1])
                                    render->DrawQuad2D(SBPageCSpellsTextureList[index + 1], {pX_coord, pY_coord});
                            } else {
                                render->DrawQuad2D(pTexture, {pX_coord, pY_coord});
                            }
                        }
                    }
                }
            }
        }
    }
}

GUIWindow_Spellbook::~GUIWindow_Spellbook() {
    onCloseSpellBookPage();
    onCloseSpellBook();
}

void GUIWindow_Spellbook::loadSpellbook() {
    MagicSchool page = pParty->activeCharacter().lastOpenedSpellbookPage;
    if (pParty->activeCharacter().uQuickSpell != SPELL_NONE && magicSchoolForSpell(pParty->activeCharacter().uQuickSpell) == page) {
        spellbookSelectedSpell = pParty->activeCharacter().uQuickSpell;
    } else {
        spellbookSelectedSpell = SPELL_NONE;
    }

    for (SpellId spell : spellsForMagicSchool(page)) {
        if (pParty->activeCharacter().bHaveSpell[spell] || engine->config->debug.AllMagic.value()) {
            int index = spellIndexInMagicSchool(spell);
            int spellNum = pSpellbookSpellIndices[page][index + 1];
            std::string pContainer;

            if (gameType() == GAME_TYPE_MM8) {
                // MM8's spell-icon assets in icons.lod are "sb{school}{NN}{a|b}" (lowercase, number
                // BEFORE the state letter), a
                // different naming scheme from MM7's "SB{school}{S|C}{NN}". Every school letter
                // matches MM7's table below except DARK, whose MM8 prefix is "dk" not "d"
                // ("sbd01a"/"sbde01a" are unrelated 4-frame assets, not the 11-spell DARK set -
                // "sbdk01a".."sbdk11b" is the real one).
                const char *suffix = (page == MAGIC_SCHOOL_DARK) ? "dk" : spellbook_texture_filename_suffices[page];
                pContainer = fmt::format("sb{}{:02}a", suffix, spellNum);
                SBPageSSpellsTextureList[index + 1] = assets->getImage_Solid(pContainer);

                pContainer = fmt::format("sb{}{:02}b", suffix, spellNum);
                SBPageCSpellsTextureList[index + 1] = assets->getImage_Solid(pContainer);
            } else {
                pContainer = fmt::format("SB{}S{:02}", spellbook_texture_filename_suffices[page], spellNum);
                SBPageSSpellsTextureList[index + 1] = assets->getImage_Solid(pContainer);

                pContainer = fmt::format("SB{}C{:02}", spellbook_texture_filename_suffices[page], spellNum);
                SBPageCSpellsTextureList[index + 1] = assets->getImage_Solid(pContainer);
            }
        }
    }
}

void GUIWindow_Spellbook::drawCurrentSchoolBackground() {
    MagicSchool page = MAGIC_SCHOOL_FIRE;
    if (pParty->hasActiveCharacter())
        page = pParty->activeCharacter().lastOpenedSpellbookPage;

    if (gameType() == GAME_TYPE_MM8) {
        // MM8 ships no per-school page background image, so the page is filled flat with the page and
        // spine colours of the original.
        static constexpr Color kMm8PageColor{228, 200, 135};
        static constexpr Color kMm8SpineColor{90, 50, 10};
        static constexpr Color kMm8EdgeColor{40, 20, 0};
        render->FillRect({0, 0, 640, 480}, kMm8PageColor);
        render->FillRect({0, 0, 16, 480}, kMm8EdgeColor);
        render->FillRect({624, 0, 16, 480}, kMm8EdgeColor);
        render->FillRect({312, 0, 16, 480}, kMm8SpineColor);

        // School emblem: no large badge asset exists. The school's medallion ("st{suffix}u", 48x39)
        // is upscaled to about 115x100, the size the original shows.
        if (ui_spellbook_school_tabs[page][1]) {
            std::shared_ptr<GraphicsImage> emblem = ui_spellbook_school_tabs[page][1];
            render->DrawQuad2D(emblem, {0, 0, emblem->width(), emblem->height()}, {45, 45, 115, 100});
        }
        DrawTitleText(assets->pFontBookTitle.get(), 0, 30, colorTable.Black,
                               localization->spellSchoolName(page), 0, Recti(45, 0, 115, 480));

        if (pParty->hasActiveCharacter()) {
            DrawTitleText(assets->pFontBookTitle.get(), 0, 17, colorTable.Black,
                                   pParty->activeCharacter().name, 0, Recti(380, 0, 240, 480));
        }

        // ui_spellbook_btn_quckspell is null on MM8 (see kMm8QuickSpellSize's own
        // comment above), and DrawQuad2D is a no-op on a null texture.
        render->DrawQuad2D(ui_spellbook_btn_quckspell, kMm8QuickSpellPos);
        if (ui_spellbook_unconfirmed_icon)
            render->DrawQuad2D(ui_spellbook_unconfirmed_icon, {kMm8ClosePos.x, kMm8QuickSpellPos.y + kMm8QuickSpellSize.h});
        render->DrawQuad2D(ui_spellbook_btn_close, kMm8ClosePos);
        return;
    }

    render->DrawQuad2D(ui_spellbook_school_backgrounds[page], {8, 8});

    render->DrawQuad2D(ui_spellbook_btn_quckspell, {476, 450});
    render->DrawQuad2D(ui_spellbook_btn_close, {561, 450});
}

void GUIWindow_Spellbook::initializeTextures() {
    pAudioPlayer->playUISound(SOUND_openbook);

    if (gameType() == GAME_TYPE_MM8) {
        // MM8's spellbook assets:
        //   Close              -> "stxtu"/"stxtd" (icons.lod)
        //   Quick Spell        -> "gf-aspell"/"gf-aspelld", in "00 add.icons.lod", which the engine does not open
        //   3rd left-edge icon -> "stssu"/"stssd" (icons.lod), role unknown, drawn with no click handler
        //   School tabs (9)    -> "st{suffix}d"/"st{suffix}u" (icons.lod), "u" is also the page emblem
        //   Page background    -> no image asset, see drawCurrentSchoolBackground()
        ui_spellbook_btn_close = assets->getImage_Solid("stxtu");
        ui_spellbook_btn_close_click = assets->getImage_Solid("stxtd");
        ui_spellbook_btn_quckspell = nullptr;
        ui_spellbook_btn_quckspell_click = nullptr;
        ui_spellbook_unconfirmed_icon = assets->getImage_Solid("stssu");
        // Same title font UIBooks.cpp's Calendar/Autonotes screens already load ("book2.fnt").
        if (!assets->pFontBookTitle)
            assets->pFontBookTitle = GUIFont::LoadFont("book2.fnt");

        for (MagicSchool page : allMagicSchools()) {
            const char *suffix = spellbook_texture_filename_suffices[page];
            ui_spellbook_school_tabs[page][0] = assets->getImage_Solid(fmt::format("st{}d", suffix));
            ui_spellbook_school_tabs[page][1] = assets->getImage_Solid(fmt::format("st{}u", suffix));
        }
        return;
    }

    ui_spellbook_btn_close = assets->getImage_Solid("ib-m5-u");
    ui_spellbook_btn_close_click = assets->getImage_Solid("ib-m5-d");
    ui_spellbook_btn_quckspell = assets->getImage_Solid("ib-m6-u");
    ui_spellbook_btn_quckspell_click = assets->getImage_Solid("ib-m6-d");

    for (MagicSchool page : allMagicSchools()) {
        ui_spellbook_school_backgrounds[page] = assets->getIcon(texNames[page]);
        ui_spellbook_school_tabs[page][0] = assets->getIcon(fmt::format("tab{}a", std::to_underlying(page) + 1));
        ui_spellbook_school_tabs[page][1] = assets->getIcon(fmt::format("tab{}b", std::to_underlying(page) + 1));
    }
}

void GUIWindow_Spellbook::onCloseSpellBook() {
    pAudioPlayer->playUISound(SOUND_closebook);
}

void GUIWindow_Spellbook::onCloseSpellBookPage() {
    SBPageCSpellsTextureList.fill(nullptr);
    SBPageSSpellsTextureList.fill(nullptr);

    DeleteButtons();
}
