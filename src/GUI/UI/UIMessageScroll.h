#pragma once

#include <string>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/ItemEnumFunctions.h"

#include "GUI/GUIEnums.h"
#include "GUI/GUIWindow.h"

class GUIWindow_MessageScroll : public GUIWindow {
 public:
    GUIWindow_MessageScroll(Pointi position, Sizei dimensions, ItemId scroll_type, std::string_view hint = {}) :
        GUIWindow(WINDOW_Scroll, position, dimensions, hint) {
        assert(isMessageScroll(scroll_type));

        this->scroll_type = scroll_type;
        CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, INPUT_ACTION_SELECT_CHAR_1, "");
        CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, INPUT_ACTION_SELECT_CHAR_2, "");
        CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, INPUT_ACTION_SELECT_CHAR_3, "");
        CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, INPUT_ACTION_SELECT_CHAR_4, "");
        CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, INPUT_ACTION_NEXT_CHAR, "");
    }
    virtual ~GUIWindow_MessageScroll() {}

    virtual void Update() override;

    ItemId scroll_type = ITEM_NULL;
};

/**
 * @offset 0x467F48
 */
void CreateMsgScrollWindow(ItemId mscroll_id);
