#include "UISpell.h"

#include <cassert>

#include "Engine/Engine.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Timer.h"

#include "Io/Mouse.h"

#include "UIStatusBar.h"
#include "Engine/Graphics/Viewport.h"

TargetedSpellUI::TargetedSpellUI(WindowType windowType, Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint)
    : GUIWindow(windowType, position, dimensions, hint), _spellInfo(spellInfo) {
    assert(spellInfo);

    gameTimer->setPaused(true);
    mouse->SetCursorImage("MICON2");
    engine->_statusBar->setEvent(LSTR_SELECT_TARGET);
}

TargetedSpellUI_Hirelings::TargetedSpellUI_Hirelings(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint)
    : TargetedSpellUI(WINDOW_CastSpell, position, dimensions, spellInfo, hint) {
    CreateButton({469, 178}, ui_btn_npc_left->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_ScrollNPCPanel, 0, INPUT_ACTION_INVALID, "", {ui_btn_npc_left});
    CreateButton({626, 178}, ui_btn_npc_right->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_ScrollNPCPanel, 1, INPUT_ACTION_INVALID, "", {ui_btn_npc_right});
    CreateButton({491, 149}, {64, 74}, BUTTON_TYPE_NORMAL, 0, UIMSG_CastSpell_Hireling, 0, INPUT_ACTION_SELECT_NPC_1);
    CreateButton({561, 149}, {64, 74}, BUTTON_TYPE_NORMAL, 0, UIMSG_CastSpell_Hireling, 1, INPUT_ACTION_SELECT_NPC_2);
}

TargetedSpellUI_Character::TargetedSpellUI_Character(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint)
    : TargetedSpellUI(WINDOW_CastSpell, position, dimensions, spellInfo, hint) {
    CreateButtonsTargetCharacters();
}

TargetedSpellUI_Actor::TargetedSpellUI_Actor(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint)
    : TargetedSpellUI(WINDOW_CastSpell, position, dimensions, spellInfo, hint) {
    CreateButton(pViewport.topLeft(), pViewport.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_CastSpell_TargetActor, 0);
}

TargetedSpellUI_ActorOrCharacter::TargetedSpellUI_ActorOrCharacter(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint)
    : TargetedSpellUI(WINDOW_CastSpell, position, dimensions, spellInfo, hint) {
    CreateButtonsTargetCharacters();
    CreateButton(pViewport.topLeft(), pViewport.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_CastSpell_TargetActorBuff, 0);
}

TargetedSpellUI_Telekinesis::TargetedSpellUI_Telekinesis(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint)
    : TargetedSpellUI(WINDOW_CastSpell, position, dimensions, spellInfo, hint) {
    CreateButton(pViewport.topLeft(), pViewport.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_CastSpell_Telekinesis, 0);
}

void TargetedSpellUI::CreateButtonsTargetCharacters() {
    // TODO(pskelton): why is position / size different to normal character buttons. These are 72 by 72 circles
    // where the game portraits are 64 by 82 ovals, and their centers sit 18, 16, 16 and 11 pixels right of the
    // portrait's left edge while the game buttons hold a constant 27 to 28. That offset has to be the same for
    // all four, so these numbers look mis-transcribed rather than deliberate.
    CreateButton({16, 386}, {71, 71}, BUTTON_TYPE_CHARACTER, 0, UIMSG_CastSpell_TargetCharacter, 0, INPUT_ACTION_SELECT_CHAR_1);
    CreateButton({129, 386}, {71, 71}, BUTTON_TYPE_CHARACTER, 0, UIMSG_CastSpell_TargetCharacter, 1, INPUT_ACTION_SELECT_CHAR_2);
    CreateButton({244, 386}, {71, 71}, BUTTON_TYPE_CHARACTER, 0, UIMSG_CastSpell_TargetCharacter, 2, INPUT_ACTION_SELECT_CHAR_3);
    CreateButton({354, 386}, {71, 71}, BUTTON_TYPE_CHARACTER, 0, UIMSG_CastSpell_TargetCharacter, 3, INPUT_ACTION_SELECT_CHAR_4);
}
