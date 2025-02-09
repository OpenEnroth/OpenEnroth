#include "ImGuiBindings.h"

#include <imgui/imgui.h>
#include <string>
#include <vector>
#include <tuple>
#include <sol/sol.hpp>

// Windows
bool imGuiBegin(const std::string &name) { return ImGui::Begin(name.c_str()); }
std::tuple<bool, bool> imGuiBeginEx(const std::string &name, bool isOpened, ImGuiWindowFlags_ flags) {
    bool shouldDraw = ImGui::Begin(name.c_str(), &isOpened, flags);
    return { shouldDraw, isOpened };
}
void imGuiEnd() { ImGui::End(); }

// Child Windows
bool imGuiBeginChild(const std::string &name, float sizeX, float sizeY, bool border) { return ImGui::BeginChild(name.c_str(), { sizeX, sizeY }, border); }
void imGuiEndChild() { ImGui::EndChild(); }

// Windows Utilities
bool imGuiIsWindowHovered(ImGuiHoveredFlags_ flags) { return ImGui::IsWindowHovered(flags); }
std::tuple<float, float> imGuiGetWindowSize() { const auto vec2{ ImGui::GetWindowSize() };  return std::make_tuple(vec2.x, vec2.y); }
void imGuiSetNextWindowPos(float posX, float posY) { ImGui::SetNextWindowPos({ posX, posY }); }
void imGuiSetNextWindowPosEx(float posX, float posY, ImGuiCond_ cond) { ImGui::SetNextWindowPos({ posX, posY }, cond); }
void imGuiSetNextWindowSize(float sizeX, float sizeY) { ImGui::SetNextWindowSize({ sizeX, sizeY }); }
void imGuiSetNextWindowSizeEx(float sizeX, float sizeY, ImGuiCond_ cond) { ImGui::SetNextWindowSize({ sizeX, sizeY }, cond); }
void imGuiSetNextWindowSizeConstraints(float minX, float minY, float maxX, float maxY) { ImGui::SetNextWindowSizeConstraints({ minX, minY }, { maxX, maxY }); }

// Windows Scrolling
void imGuiSetScrollHereY(float y) { ImGui::SetScrollHereY(y); }

// Styles
void imGuiPushStyleColor(ImGuiCol_ colorType, float r, float g, float b, float a) { ImGui::PushStyleColor(colorType, { r, g, b, a }); }
void imGuiPopStyleColor() { ImGui::PopStyleColor(); }

// Widgets: Text
void imGuiTextUnformatted(const std::string &text) { ImGui::TextUnformatted(text.c_str()); }
void imGuiText(const std::string &text) { ImGui::Text("%s", text.c_str()); }
void imGuiTextWrapped(const std::string text) { ImGui::TextWrapped("%s", text.c_str()); }

// Widgets: Common
bool imGuiButton(const std::string &label) { return ImGui::Button(label.c_str()); }
bool imGuiButtonEx(const std::string &label, float sizeX, float sizeY) { return ImGui::Button(label.c_str(), { sizeX, sizeY }); }
std::tuple<bool, bool> imGuiCheckbox(const std::string &label, bool value) {
    bool pressed = ImGui::Checkbox(label.c_str(), &value);
    return { value, pressed };
}

// Tables
void imGuiBeginTable(std::string name, int columns) { ImGui::BeginTable(name.c_str(), columns); }
void imGuiEndTable() { ImGui::EndTable(); }
void imGuiTableNextRow() { ImGui::TableNextRow(); }
void imGuiTableSetColumnIndex(int index) { ImGui::TableSetColumnIndex(index); }
int imGuiTableGetColumnCount() { return ImGui::TableGetColumnCount(); }

// Item/Widgets Utilities
bool imGuiIsItemFocused() { return ImGui::IsItemFocused(); }

// Inputs
bool imGuiIsMouseHoveringRect(float minX, float minY, float maxX, float maxY) { return ImGui::IsMouseHoveringRect({ minX, minY }, { maxX, maxY }); }

// Layout
void imGuiSameLine() { ImGui::SameLine(); }
float imGuiGetFrameHeightWithSpacing() { return ImGui::GetFrameHeightWithSpacing(); }

// Inputs Utilities: Keyboard
void imGuiSetKeyboardFocusHere(int offset) { ImGui::SetKeyboardFocusHere(offset); }

// Widgets: Input with Keyboard
struct ImGuiInputTextUserData {
    void onResize(ImGuiInputTextCallbackData *data) {
        string->resize(data->BufTextLen);
        data->Buf = string->data();
    }

    void onHistory(ImGuiInputTextCallbackData *data) {
        if (!callback) return;
        int step = 0;
        switch (data->EventKey) {
        case ImGuiKey_UpArrow:
            step = -1;
            break;
        case ImGuiKey_DownArrow:
            step = 1;
            break;
        default:
            break;
        }

        *string = callback->call(data->EventFlag, step);
        data->DeleteChars(0, data->BufTextLen);
        data->InsertChars(0, string->data());
    }

    std::string *string{};
    sol::function *callback{};
};

int inputTextCallback(ImGuiInputTextCallbackData *data) {
    switch (data->EventFlag) {
    case ImGuiInputTextFlags_CallbackResize: {
        ImGuiInputTextUserData *userData = static_cast<ImGuiInputTextUserData *>(data->UserData);
        userData->onResize(data);
        break;
    }
    case ImGuiInputTextFlags_CallbackHistory: {
        ImGuiInputTextUserData *userData = static_cast<ImGuiInputTextUserData *>(data->UserData);
        userData->onHistory(data);
        break;
    }
    }

    return 0;
}

std::tuple<std::string, bool> imGuiInputTextWithHint(const std::string &label, const std::string &hint, std::string text, ImGuiInputTextFlags flags, sol::function callback) {
    ImGuiInputTextUserData userData{ &text, callback.valid() ? &callback : nullptr };
    bool selected = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), text.data(), text.capacity() + 1, flags | ImGuiInputTextFlags_CallbackResize, inputTextCallback, &userData);
    return { text, selected };
}

void InitEnums(sol::table &table) {
    table.new_enum("ImGuiWindowFlags",
        "None", ImGuiWindowFlags_None,
        "NoTitleBar", ImGuiWindowFlags_NoTitleBar,
        "NoResize", ImGuiWindowFlags_NoResize,
        "NoMove", ImGuiWindowFlags_NoMove,
        "NoScrollbar", ImGuiWindowFlags_NoScrollbar,
        "NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse,
        "NoCollapse", ImGuiWindowFlags_NoCollapse,
        "AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize,
        "NoBackground", ImGuiWindowFlags_NoBackground,
        "NoSavedSettings", ImGuiWindowFlags_NoSavedSettings,
        "NoMouseInputs", ImGuiWindowFlags_NoMouseInputs,
        "MenuBar", ImGuiWindowFlags_MenuBar,
        "HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar,
        "NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing,
        "NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus,
        "AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar,
        "AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar,
        "AlwaysUseWindowPadding", ImGuiWindowFlags_AlwaysUseWindowPadding,
        "NoNavInputs", ImGuiWindowFlags_NoNavInputs,
        "NoNavFocus", ImGuiWindowFlags_NoNavFocus,
        "UnsavedDocument", ImGuiWindowFlags_UnsavedDocument,
        "NoDocking", ImGuiWindowFlags_NoDocking,

        "NoNav", ImGuiWindowFlags_NoNav,
        "NoDecoration", ImGuiWindowFlags_NoDecoration,
        "NoInputs", ImGuiWindowFlags_NoInputs,

        "NavFlattened", ImGuiWindowFlags_NavFlattened,
        "ChildWindow", ImGuiWindowFlags_ChildWindow,
        "Tooltip", ImGuiWindowFlags_Tooltip,
        "Popup", ImGuiWindowFlags_Popup,
        "Modal", ImGuiWindowFlags_Modal,
        "ChildMenu", ImGuiWindowFlags_ChildMenu,
        "DockNodeHost", ImGuiWindowFlags_DockNodeHost
    );

    table.new_enum("ImGuiFocusedFlags",
        "None", ImGuiFocusedFlags_None,
        "ChildWindows", ImGuiFocusedFlags_ChildWindows,
        "RootWindow", ImGuiFocusedFlags_RootWindow,
        "AnyWindow", ImGuiFocusedFlags_AnyWindow,
        "RootAndChildWindows", ImGuiFocusedFlags_RootAndChildWindows
    );

    table.new_enum("ImGuiHoveredFlags",
        "None", ImGuiHoveredFlags_None,
        "ChildWindows", ImGuiHoveredFlags_ChildWindows,
        "RootWindow", ImGuiHoveredFlags_RootWindow,
        "AnyWindow", ImGuiHoveredFlags_AnyWindow,
        "AllowWhenBlockedByPopup", ImGuiHoveredFlags_AllowWhenBlockedByPopup,
        "AllowWhenBlockedByActiveItem", ImGuiHoveredFlags_AllowWhenBlockedByActiveItem,
        "AllowWhenOverlapped", ImGuiHoveredFlags_AllowWhenOverlapped,
        "AllowWhenDisabled", ImGuiHoveredFlags_AllowWhenDisabled,
        "RectOnly", ImGuiHoveredFlags_RectOnly,
        "RootAndChildWindows", ImGuiHoveredFlags_RootAndChildWindows
    );

    table.new_enum("ImGuiCond",
        "None", ImGuiCond_None,
        "Always", ImGuiCond_Always,
        "Once", ImGuiCond_Once,
        "FirstUseEver", ImGuiCond_FirstUseEver,
        "Appearing", ImGuiCond_Appearing
    );

    table.new_enum("ImGuiCol",
        "Text", ImGuiCol_Text,
        "TextDisabled", ImGuiCol_TextDisabled,
        "WindowBg", ImGuiCol_WindowBg,
        "ChildBg", ImGuiCol_ChildBg,
        "PopupBg", ImGuiCol_PopupBg,
        "Border", ImGuiCol_Border,
        "BorderShadow", ImGuiCol_BorderShadow,
        "FrameBg", ImGuiCol_FrameBg,
        "FrameBgHovered", ImGuiCol_FrameBgHovered,
        "FrameBgActive", ImGuiCol_FrameBgActive,
        "TitleBg", ImGuiCol_TitleBg,
        "TitleBgActive", ImGuiCol_TitleBgActive,
        "TitleBgCollapsed", ImGuiCol_TitleBgCollapsed,
        "MenuBarBg", ImGuiCol_MenuBarBg,
        "ScrollbarBg", ImGuiCol_ScrollbarBg,
        "ScrollbarGrab", ImGuiCol_ScrollbarGrab,
        "ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered,
        "ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive,
        "CheckMark", ImGuiCol_CheckMark,
        "SliderGrab", ImGuiCol_SliderGrab,
        "SliderGrabActive", ImGuiCol_SliderGrabActive,
        "Button", ImGuiCol_Button,
        "ButtonHovered", ImGuiCol_ButtonHovered,
        "ButtonActive", ImGuiCol_ButtonActive,
        "Header", ImGuiCol_Header,
        "HeaderHovered", ImGuiCol_HeaderHovered,
        "HeaderActive", ImGuiCol_HeaderActive,
        "Separator", ImGuiCol_Separator,
        "SeparatorHovered", ImGuiCol_SeparatorHovered,
        "SeparatorActive", ImGuiCol_SeparatorActive,
        "ResizeGrip", ImGuiCol_ResizeGrip,
        "ResizeGripHovered", ImGuiCol_ResizeGripHovered,
        "ResizeGripActive", ImGuiCol_ResizeGripActive,
        "Tab", ImGuiCol_Tab,
        "TabHovered", ImGuiCol_TabHovered,
        "TabActive", ImGuiCol_TabActive,
        "TabUnfocused", ImGuiCol_TabUnfocused,
        "TabUnfocusedActive", ImGuiCol_TabUnfocusedActive,
        "DockingPreview", ImGuiCol_DockingPreview,
        "DockingEmptyBg", ImGuiCol_DockingEmptyBg,
        "PlotLines", ImGuiCol_PlotLines,
        "PlotLinesHovered", ImGuiCol_PlotLinesHovered,
        "PlotHistogram", ImGuiCol_PlotHistogram,
        "PlotHistogramHovered", ImGuiCol_PlotHistogramHovered,
        "TableHeaderBg", ImGuiCol_TableHeaderBg,
        "TableBorderStrong", ImGuiCol_TableBorderStrong,
        "TableBorderLight", ImGuiCol_TableBorderLight,
        "TableRowBg", ImGuiCol_TableRowBg,
        "TableRowBgAlt", ImGuiCol_TableRowBgAlt,
        "TextSelectedBg", ImGuiCol_TextSelectedBg,
        "DragDropTarget", ImGuiCol_DragDropTarget,
        "NavHighlight", ImGuiCol_NavHighlight,
        "NavWindowingHighlight", ImGuiCol_NavWindowingHighlight,
        "NavWindowingDimBg", ImGuiCol_NavWindowingDimBg,
        "ModalWindowDimBg", ImGuiCol_ModalWindowDimBg
    );

    table.new_enum("ImGuiStyleVar",
        "Alpha", ImGuiStyleVar_Alpha,
        "WindowPadding", ImGuiStyleVar_WindowPadding,
        "WindowRounding", ImGuiStyleVar_WindowRounding,
        "WindowBorderSize", ImGuiStyleVar_WindowBorderSize,
        "WindowMinSize", ImGuiStyleVar_WindowMinSize,
        "WindowTitleAlign", ImGuiStyleVar_WindowTitleAlign,
        "ChildRounding", ImGuiStyleVar_ChildRounding,
        "ChildBorderSize", ImGuiStyleVar_ChildBorderSize,
        "PopupRounding", ImGuiStyleVar_PopupRounding,
        "PopupBorderSize", ImGuiStyleVar_PopupBorderSize,
        "FramePadding", ImGuiStyleVar_FramePadding,
        "FrameRounding", ImGuiStyleVar_FrameRounding,
        "FrameBorderSize", ImGuiStyleVar_FrameBorderSize,
        "ItemSpacing", ImGuiStyleVar_ItemSpacing,
        "ItemInnerSpacing", ImGuiStyleVar_ItemInnerSpacing,
        "IndentSpacing", ImGuiStyleVar_IndentSpacing,
        "ScrollbarSize", ImGuiStyleVar_ScrollbarSize,
        "ScrollbarRounding", ImGuiStyleVar_ScrollbarRounding,
        "GrabMinSize", ImGuiStyleVar_GrabMinSize,
        "GrabRounding", ImGuiStyleVar_GrabRounding,
        "TabRounding", ImGuiStyleVar_TabRounding,
        "SelectableTextAlign", ImGuiStyleVar_SelectableTextAlign,
        "ButtonTextAlign", ImGuiStyleVar_ButtonTextAlign,
        "COUNT", ImGuiStyleVar_COUNT
    );

    table.new_enum("ImGuiDir",
        "None", ImGuiDir_None,
        "Left", ImGuiDir_Left,
        "Right", ImGuiDir_Right,
        "Up", ImGuiDir_Up,
        "Down", ImGuiDir_Down,
        "COUNT", ImGuiDir_COUNT
    );

    table.new_enum("ImGuiComboFlags",
        "None", ImGuiComboFlags_None,
        "PopupAlignLeft", ImGuiComboFlags_PopupAlignLeft,
        "HeightSmall", ImGuiComboFlags_HeightSmall,
        "HeightRegular", ImGuiComboFlags_HeightRegular,
        "HeightLarge", ImGuiComboFlags_HeightLarge,
        "HeightLargest", ImGuiComboFlags_HeightLargest,
        "NoArrowButton", ImGuiComboFlags_NoArrowButton,
        "NoPreview", ImGuiComboFlags_NoPreview,
        "HeightMask", ImGuiComboFlags_HeightMask_
    );

    table.new_enum("ImGuiInputTextFlags",
        "None", ImGuiInputTextFlags_None,
        "CharsDecimal", ImGuiInputTextFlags_CharsDecimal,
        "CharsHexadecimal", ImGuiInputTextFlags_CharsHexadecimal,
        "CharsUppercase", ImGuiInputTextFlags_CharsUppercase,
        "CharsNoBlank", ImGuiInputTextFlags_CharsNoBlank,
        "AutoSelectAll", ImGuiInputTextFlags_AutoSelectAll,
        "EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue,
        "CallbackCompletion", ImGuiInputTextFlags_CallbackCompletion,
        "CallbackHistory", ImGuiInputTextFlags_CallbackHistory,
        "CallbackAlways", ImGuiInputTextFlags_CallbackAlways,
        "CallbackCharFilter", ImGuiInputTextFlags_CallbackCharFilter,
        "AllowTabInput", ImGuiInputTextFlags_AllowTabInput,
        "CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine,
        "NoHorizontalScroll", ImGuiInputTextFlags_NoHorizontalScroll,
        "AlwaysOverwrite", ImGuiInputTextFlags_AlwaysOverwrite,
        "ReadOnly", ImGuiInputTextFlags_ReadOnly,
        "Password", ImGuiInputTextFlags_Password,
        "NoUndoRedo", ImGuiInputTextFlags_NoUndoRedo,
        "CharsScientific", ImGuiInputTextFlags_CharsScientific,
        "CallbackResize", ImGuiInputTextFlags_CallbackResize
    );

    table.new_enum("ImGuiColorEditFlags",
        "None", ImGuiColorEditFlags_None,
        "NoAlpha", ImGuiColorEditFlags_NoAlpha,
        "NoPicker", ImGuiColorEditFlags_NoPicker,
        "NoOptions", ImGuiColorEditFlags_NoOptions,
        "NoSmallPreview", ImGuiColorEditFlags_NoSmallPreview,
        "NoInputs", ImGuiColorEditFlags_NoInputs,
        "NoTooltip", ImGuiColorEditFlags_NoTooltip,
        "NoLabel", ImGuiColorEditFlags_NoLabel,
        "NoSidePreview", ImGuiColorEditFlags_NoSidePreview,
        "NoDragDrop", ImGuiColorEditFlags_NoDragDrop,
        "NoBorder", ImGuiColorEditFlags_NoBorder,

        "AlphaBar", ImGuiColorEditFlags_AlphaBar,
        "AlphaPreview", ImGuiColorEditFlags_AlphaPreview,
        "AlphaPreviewHalf", ImGuiColorEditFlags_AlphaPreviewHalf,
        "HDR", ImGuiColorEditFlags_HDR,
        "DisplayRGB", ImGuiColorEditFlags_DisplayRGB,
        "DisplayHSV", ImGuiColorEditFlags_DisplayHSV,
        "DisplayHex", ImGuiColorEditFlags_DisplayHex,
        "Uint8", ImGuiColorEditFlags_Uint8,
        "Float", ImGuiColorEditFlags_Float,
        "PickerHueBar", ImGuiColorEditFlags_PickerHueBar,
        "PickerHueWheel", ImGuiColorEditFlags_PickerHueWheel,
        "InputRGB", ImGuiColorEditFlags_InputRGB,
        "InputHSV", ImGuiColorEditFlags_InputHSV
    );

    table.new_enum("ImGuiTreeNodeFlags",
        "None", ImGuiTreeNodeFlags_None,
        "Selected", ImGuiTreeNodeFlags_Selected,
        "Framed", ImGuiTreeNodeFlags_Framed,
        "AllowItemOverlap", ImGuiTreeNodeFlags_AllowItemOverlap,
        "NoTreePushOnOpen", ImGuiTreeNodeFlags_NoTreePushOnOpen,
        "NoAutoOpenOnLog", ImGuiTreeNodeFlags_NoAutoOpenOnLog,
        "DefaultOpen", ImGuiTreeNodeFlags_DefaultOpen,
        "OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick,
        "OpenOnArrow", ImGuiTreeNodeFlags_OpenOnArrow,
        "Leaf", ImGuiTreeNodeFlags_Leaf,
        "Bullet", ImGuiTreeNodeFlags_Bullet,
        "FramePadding", ImGuiTreeNodeFlags_FramePadding,
        "SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth,
        "SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth,
        "NavLeftJumpsBackHere", ImGuiTreeNodeFlags_NavLeftJumpsBackHere,
        "CollapsingHeader", ImGuiTreeNodeFlags_CollapsingHeader
    );

    table.new_enum("ImGuiSelectableFlags",
        "None", ImGuiSelectableFlags_None,
        "DontClosePopups", ImGuiSelectableFlags_DontClosePopups,
        "SpanAllColumns", ImGuiSelectableFlags_SpanAllColumns,
        "AllowDoubleClick", ImGuiSelectableFlags_AllowDoubleClick,
        "Disabled", ImGuiSelectableFlags_Disabled,
        "AllowItemOverlap", ImGuiSelectableFlags_AllowItemOverlap
    );

    table.new_enum("ImGuiPopupFlags",
        "None", ImGuiPopupFlags_None,
        "MouseButtonLeft", ImGuiPopupFlags_MouseButtonLeft,
        "MouseButtonRight", ImGuiPopupFlags_MouseButtonRight,
        "MouseButtonMiddle", ImGuiPopupFlags_MouseButtonMiddle,
        "MouseButtonMask_", ImGuiPopupFlags_MouseButtonMask_,
        "MouseButtonDefault_", ImGuiPopupFlags_MouseButtonDefault_,
        "NoOpenOverExistingPopup", ImGuiPopupFlags_NoOpenOverExistingPopup,
        "NoOpenOverItems", ImGuiPopupFlags_NoOpenOverItems,
        "AnyPopupId", ImGuiPopupFlags_AnyPopupId,
        "AnyPopupLevel", ImGuiPopupFlags_AnyPopupLevel,
        "AnyPopup", ImGuiPopupFlags_AnyPopup
    );

    table.new_enum("ImGuiTabBarFlags",
        "None", ImGuiTabBarFlags_None,
        "Reorderable", ImGuiTabBarFlags_Reorderable,
        "AutoSelectNewTabs", ImGuiTabBarFlags_AutoSelectNewTabs,
        "TabListPopupButton", ImGuiTabBarFlags_TabListPopupButton,
        "NoCloseWithMiddleMouseButton", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton,
        "NoTabListScrollingButtons", ImGuiTabBarFlags_NoTabListScrollingButtons,
        "NoTooltip", ImGuiTabBarFlags_NoTooltip,
        "FittingPolicyResizeDown", ImGuiTabBarFlags_FittingPolicyResizeDown,
        "FittingPolicyScroll", ImGuiTabBarFlags_FittingPolicyScroll,
        "FittingPolicyMask_", ImGuiTabBarFlags_FittingPolicyMask_,
        "FittingPolicyDefault_", ImGuiTabBarFlags_FittingPolicyDefault_
    );

    table.new_enum("ImGuiTabItemFlags",
        "None", ImGuiTabItemFlags_None,
        "UnsavedDocument", ImGuiTabItemFlags_UnsavedDocument,
        "SetSelected", ImGuiTabItemFlags_SetSelected,
        "NoCloseWithMiddleMouseButton", ImGuiTabItemFlags_NoCloseWithMiddleMouseButton,
        "NoPushId", ImGuiTabItemFlags_NoPushId,
        "NoTooltip", ImGuiTabItemFlags_NoTooltip
    );

    table.new_enum("ImGuiDockNodeFlags",
        "None", ImGuiDockNodeFlags_None,
        "KeepAliveOnly", ImGuiDockNodeFlags_KeepAliveOnly,
        "NoDockingInCentralNode", ImGuiDockNodeFlags_NoDockingInCentralNode,
        "PassthruCentralNode", ImGuiDockNodeFlags_PassthruCentralNode,
        "NoSplit", ImGuiDockNodeFlags_NoSplit,
        "NoResize", ImGuiDockNodeFlags_NoResize,
        "AutoHideTabBar", ImGuiDockNodeFlags_AutoHideTabBar
    );

    table.new_enum("ImGuiMouseButton",
        "ImGuiMouseButton_Left", ImGuiMouseButton_Left,
        "ImGuiMouseButton_Right", ImGuiMouseButton_Right,
        "ImGuiMouseButton_Middle", ImGuiMouseButton_Middle,
        "ImGuiMouseButton_COUNT", ImGuiMouseButton_COUNT
    );

    table.new_enum("ImGuiKey",
        "Tab", ImGuiKey_Tab,
        "LeftArrow", ImGuiKey_LeftArrow,
        "RightArrow", ImGuiKey_RightArrow,
        "UpArrow", ImGuiKey_UpArrow,
        "DownArrow", ImGuiKey_DownArrow,
        "PageUp", ImGuiKey_PageUp,
        "PageDown", ImGuiKey_PageDown,
        "Home", ImGuiKey_Home,
        "End", ImGuiKey_End,
        "Insert", ImGuiKey_Insert,
        "Delete", ImGuiKey_Delete,
        "Backspace", ImGuiKey_Backspace,
        "Space", ImGuiKey_Space,
        "Enter", ImGuiKey_Enter,
        "Escape", ImGuiKey_Escape,
        "A", ImGuiKey_A,
        "C", ImGuiKey_C,
        "V", ImGuiKey_V,
        "X", ImGuiKey_X,
        "Y", ImGuiKey_Y,
        "Z", ImGuiKey_Z,
        "COUNT", ImGuiKey_COUNT
    );

    table.new_enum("ImGuiMouseCursor",
        "None", ImGuiMouseCursor_None,
        "Arrow", ImGuiMouseCursor_Arrow,
        "TextInput", ImGuiMouseCursor_TextInput,
        "ResizeAll", ImGuiMouseCursor_ResizeAll,
        "ResizeNS", ImGuiMouseCursor_ResizeNS,
        "ResizeEW", ImGuiMouseCursor_ResizeEW,
        "ResizeNESW", ImGuiMouseCursor_ResizeNESW,
        "ResizeNWSE", ImGuiMouseCursor_ResizeNWSE,
        "Hand", ImGuiMouseCursor_Hand,
        "NotAllowed", ImGuiMouseCursor_NotAllowed,
        "COUNT", ImGuiMouseCursor_COUNT
    );
}

void ImGuiBindings::Init(sol::state_view &solState, sol::table &table) {
    sol::table ImGui = solState.create_table();
    table["imgui"] = ImGui;
    InitEnums(ImGui);

    ImGui.set_function("showDemoWindow", []() { ImGui::ShowDemoWindow(); });

    ImGui.set_function("beginWindow", sol::overload(imGuiBegin, imGuiBeginEx));
    ImGui.set_function("endWindow", imGuiEnd);

    ImGui.set_function("beginChild", imGuiBeginChild);
    ImGui.set_function("endChild", imGuiEndChild);

    ImGui.set_function("isWindowHovered", imGuiIsWindowHovered);
    ImGui.set_function("getWindowSize", imGuiGetWindowSize);

    ImGui.set_function("setNextWindowPos", sol::overload(imGuiSetNextWindowPos, imGuiSetNextWindowPosEx));
    ImGui.set_function("setNextWindowSize", sol::overload(imGuiSetNextWindowSize, imGuiSetNextWindowSizeEx));
    ImGui.set_function("setNextWindowSizeConstraints", imGuiSetNextWindowSizeConstraints);

    ImGui.set_function("setScrollHereY", imGuiSetScrollHereY);

    ImGui.set_function("pushStyleColor", imGuiPushStyleColor);
    ImGui.set_function("popStyleColor", imGuiPopStyleColor);

    ImGui.set_function("textUnformatted", imGuiTextUnformatted);
    ImGui.set_function("text", imGuiText);
    ImGui.set_function("textWrapped", imGuiTextWrapped);

    ImGui.set_function("button", sol::overload(imGuiButton, imGuiButtonEx));
    ImGui.set_function("checkbox", imGuiCheckbox);

    ImGui.set_function("inputTextWithHint", imGuiInputTextWithHint);

    ImGui.set_function("beginTable", imGuiBeginTable);
    ImGui.set_function("endTable", imGuiEndTable);
    ImGui.set_function("tableNextRow", imGuiTableNextRow);
    ImGui.set_function("tableSetColumnIndex", imGuiTableSetColumnIndex);
    ImGui.set_function("tableGetColumnCount", imGuiTableGetColumnCount);

    ImGui.set_function("isItemFocused", imGuiIsItemFocused);

    ImGui.set_function("setKeyboardFocusHere", imGuiSetKeyboardFocusHere);

    ImGui.set_function("sameLine", imGuiSameLine);
    ImGui.set_function("getFrameHeightWithSpacing", imGuiGetFrameHeightWithSpacing);

    ImGui.set_function("isMouseHoveringRect", imGuiIsMouseHoveringRect);
}
