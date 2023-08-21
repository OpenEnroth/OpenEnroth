#include "UISpell.h"

#include <memory>

#include "UIStatusBar.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/mm7_data.h"
#include "Io/Mouse.h"
#include "Engine/Time.h"
#include "GUI/GUIEnums.h"
#include "Io/InputAction.h"

TargetedSpellUI::TargetedSpellUI(Pointi position, Sizei dimensions, WindowData data, const std::string &hint)
    : GUIWindow(WINDOW_CastSpell, position, dimensions, data, hint) {
    pEventTimer->Pause();
    mouse->SetCursorImage("MICON2");
    engine->_statusBar->setEvent(LSTR_CHOOSE_TARGET);
}

TargetedSpellUI_Hirelings::TargetedSpellUI_Hirelings(Pointi position, Sizei dimensions, WindowData data, const std::string &hint)
    : TargetedSpellUI(position, dimensions, data, hint) {
    CreateButton({469, 178}, ui_btn_npc_left->size(), 1, 0, UIMSG_ScrollNPCPanel, 0, Io::InputAction::Invalid, "", {ui_btn_npc_left});
    CreateButton({626, 178}, ui_btn_npc_right->size(), 1, 0, UIMSG_ScrollNPCPanel, 1, Io::InputAction::Invalid, "", {ui_btn_npc_right});
    CreateButton({491, 149}, {64, 74}, 1, 0, UIMSG_CastSpell_Hireling, 0, Io::InputAction::SelectNPC1);
    CreateButton({561, 149}, {64, 74}, 1, 0, UIMSG_CastSpell_Hireling, 1, Io::InputAction::SelectNPC2);
}

TargetedSpellUI_Character::TargetedSpellUI_Character(Pointi position, Sizei dimensions, WindowData data, const std::string &hint)
    : TargetedSpellUI(position, dimensions, data, hint) {
    CreateButton({52, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_TargetCharacter, 0, Io::InputAction::SelectChar1);
    CreateButton({165, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_TargetCharacter, 1, Io::InputAction::SelectChar2);
    CreateButton({280, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_TargetCharacter, 2, Io::InputAction::SelectChar3);
    CreateButton({390, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_TargetCharacter, 3, Io::InputAction::SelectChar4);
}

TargetedSpellUI_Actor::TargetedSpellUI_Actor(Pointi position, Sizei dimensions, WindowData data, const std::string &hint)
    : TargetedSpellUI(position, dimensions, data, hint) {
    CreateButton({game_viewport_x, game_viewport_y}, {game_viewport_width, game_viewport_height}, 1, 0, UIMSG_CastSpell_TargetActor, 0);
}

TargetedSpellUI_ActorOrCharacter::TargetedSpellUI_ActorOrCharacter(Pointi position, Sizei dimensions, WindowData data, const std::string &hint)
    : TargetedSpellUI(position, dimensions, data, hint) {
    CreateButton({52, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_TargetCharacter, 0, Io::InputAction::SelectChar1);
    CreateButton({165, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_TargetCharacter, 1, Io::InputAction::SelectChar2);
    CreateButton({280, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_TargetCharacter, 2, Io::InputAction::SelectChar3);
    CreateButton({390, 422}, {35, 0}, 2, 0, UIMSG_CastSpell_TargetCharacter, 3, Io::InputAction::SelectChar4);
    CreateButton({8, 8}, {game_viewport_width, game_viewport_height}, 1, 0, UIMSG_CastSpell_TargetActorBuff, 0);
}

TargetedSpellUI_Telekinesis::TargetedSpellUI_Telekinesis(Pointi position, Sizei dimensions, WindowData data, const std::string &hint)
    : TargetedSpellUI(position, dimensions, data, hint) {
    CreateButton({game_viewport_x, game_viewport_y}, {game_viewport_width, game_viewport_height}, 1, 0, UIMSG_CastSpell_Telekinesis, 0);
}
