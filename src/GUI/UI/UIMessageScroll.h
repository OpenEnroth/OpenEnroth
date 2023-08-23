#pragma once

#include <string>

#include "GUI/GUIEnums.h"
#include "GUI/GUIWindow.h"
#include "Engine/Objects/ItemEnums.h"

class GUIWindow_MessageScroll : public GUIWindow {
 public:
    GUIWindow_MessageScroll(Pointi position, Sizei dimensions, ItemId scroll_type, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_Scroll, position, dimensions, 0, hint) {
        assert(isMessageScroll(scroll_type));

        this->scroll_type = scroll_type;
        CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1, "");
        CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2, "");
        CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3, "");
        CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4, "");
        CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, Io::InputAction::CharCycle, "");
    }
    virtual ~GUIWindow_MessageScroll() {}

    virtual void Update() override;

    ItemId scroll_type = ITEM_NULL;
};

/**
 * @offset 0x467F48
 */
void CreateMsgScrollWindow(ItemId mscroll_id);
