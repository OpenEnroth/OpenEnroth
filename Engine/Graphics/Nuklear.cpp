#include <algorithm>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"

#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Localization.h"

#include "GUI/GUIWindow.h"
#include "Io/GameKey.h"

#include "Platform/OSWindow.h"
#include "Platform/Api.h"

#include "nuklear/nuklear_config.h"

using Io::GameKey;

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024
#define MAX_RATIO_ELEMENTS 16

lua_State *lua = nullptr;
std::shared_ptr<Nuklear> nuklear;

Nuklear::Nuklear() {
}

enum WIN_STATE {
    WINDOW_NOT_LOADED = 0,
    WINDOW_INITIALIZED,
    WINDOW_TEMPLATE_ERROR
};

struct hotkey {
    Io::GameKey key;
    int callback;
};
std::vector <struct hotkey> hotkeys;

struct img {
    Image *asset;
    struct nk_image nk;
};

struct windows {
    std::vector<struct img *> img;
    WindowType winType;
    WIN_STATE state;
    char *tmpl;
    Nuklear::NUKLEAR_MODE mode;
    
};
struct windows wins[WINDOW_DebugMenu] = {};
WindowType currentWin = WINDOW_null;

static enum lua_nk_style_type: __int32 {
    lua_nk_style_type_unknown,
    lua_nk_style_type_align,
    lua_nk_style_type_bool,
    lua_nk_style_type_callback,
    lua_nk_style_type_color,
    lua_nk_style_type_cursor,
    lua_nk_style_type_edit,
    lua_nk_style_type_flags,
    lua_nk_style_type_float,
    lua_nk_style_type_font,
    lua_nk_style_type_item,
    lua_nk_style_type_integer,
    lua_nk_style_type_symbol,
    lua_nk_style_type_vec2,
};

struct lua_nk_property {
    char *property;
    void *ptr;
    enum lua_nk_style_type type;
};

struct lua_nk_style {
    char *component;
    std::vector<struct lua_nk_property> props;
};

std::vector<struct lua_nk_style> lua_nk_styles;

#define PUSH_STYLE(element, component, property, type) element.push_back({ #property, &nuklear->ctx->style.##component##.##property, type});

std::shared_ptr<Nuklear> Nuklear::Initialize() {
    nuklear = std::make_shared<Nuklear>();

    nuklear->ctx = (struct nk_context *)malloc(sizeof(struct nk_context));
    if (!nuklear->ctx) {
        return nullptr;
    }

    for (int w = WINDOW_MainMenu; w != WINDOW_DebugMenu; w++) {
        wins[w].state = WINDOW_NOT_LOADED;
        wins[w].mode = NUKLEAR_MODE_SHARED;
    }

    /* button background */
    struct lua_nk_style style;
    style.props.clear();

    style.component = "button";
    /* button background */
    PUSH_STYLE(style.props, button, active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, button, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, button, hover, lua_nk_style_type_item);
    PUSH_STYLE(style.props, button, normal, lua_nk_style_type_item);
    /* button text */
    PUSH_STYLE(style.props, button, text_active, lua_nk_style_type_color);
    PUSH_STYLE(style.props, button, text_alignment, lua_nk_style_type_flags);
    PUSH_STYLE(style.props, button, text_background, lua_nk_style_type_color);
    PUSH_STYLE(style.props, button, text_hover, lua_nk_style_type_color);
    PUSH_STYLE(style.props, button, text_normal, lua_nk_style_type_color);
    /* button properties */
    PUSH_STYLE(style.props, button, border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, button, image_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, button, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, button, rounding, lua_nk_style_type_float);
    PUSH_STYLE(style.props, button, touch_padding, lua_nk_style_type_vec2);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "chart";
    /* chart background */
    PUSH_STYLE(style.props, chart, background, lua_nk_style_type_item);
    PUSH_STYLE(style.props, chart, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, chart, color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, chart, selected_color, lua_nk_style_type_color);
    /* chart properties */
    PUSH_STYLE(style.props, chart, border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, chart, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, chart, rounding, lua_nk_style_type_float);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "checkbox";
    /* background */
    PUSH_STYLE(style.props, checkbox, active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, checkbox, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, checkbox, hover, lua_nk_style_type_item);
    PUSH_STYLE(style.props, checkbox, normal, lua_nk_style_type_item);
    /* cursor */
    PUSH_STYLE(style.props, checkbox, cursor_hover, lua_nk_style_type_cursor);
    PUSH_STYLE(style.props, checkbox, cursor_normal, lua_nk_style_type_cursor);
    /* text */
    PUSH_STYLE(style.props, checkbox, text_active, lua_nk_style_type_color);
    PUSH_STYLE(style.props, checkbox, text_alignment, lua_nk_style_type_flags);
    PUSH_STYLE(style.props, checkbox, text_background, lua_nk_style_type_color);
    PUSH_STYLE(style.props, checkbox, text_normal, lua_nk_style_type_color);
    PUSH_STYLE(style.props, checkbox, text_hover, lua_nk_style_type_color);
    /* properties */
    PUSH_STYLE(style.props, checkbox, border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, checkbox, padding, lua_nk_style_type_float);
    PUSH_STYLE(style.props, checkbox, spacing, lua_nk_style_type_float);
    PUSH_STYLE(style.props, checkbox, touch_padding, lua_nk_style_type_vec2);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "combo";
    /* combo background */
    PUSH_STYLE(style.props, combo, active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, combo, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, combo, hover, lua_nk_style_type_item);
    PUSH_STYLE(style.props, combo, normal, lua_nk_style_type_item);
    /* combo text */
    PUSH_STYLE(style.props, combo, label_active, lua_nk_style_type_color);
    PUSH_STYLE(style.props, combo, label_normal, lua_nk_style_type_color);
    PUSH_STYLE(style.props, combo, label_hover, lua_nk_style_type_color);
    /* combo symbol */
    PUSH_STYLE(style.props, combo, symbol_active, lua_nk_style_type_symbol);
    PUSH_STYLE(style.props, combo, symbol_hover, lua_nk_style_type_symbol);
    PUSH_STYLE(style.props, combo, symbol_normal, lua_nk_style_type_symbol);
    /* combo button */
    {
        /* button background */
        PUSH_STYLE(style.props, combo.button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, combo.button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, combo.button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, combo.button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, combo.button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, combo.button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, combo.button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, combo.button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, combo.button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, combo.button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, combo.button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, combo.button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, combo.button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, combo.button, touch_padding, lua_nk_style_type_vec2);
    }
    PUSH_STYLE(style.props, combo, sym_active, lua_nk_style_type_symbol);
    PUSH_STYLE(style.props, combo, sym_hover, lua_nk_style_type_symbol);
    PUSH_STYLE(style.props, combo, sym_normal, lua_nk_style_type_symbol);
    /* combo properties */
    PUSH_STYLE(style.props, combo, border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, combo, button_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, combo, content_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, combo, rounding, lua_nk_style_type_float);
    PUSH_STYLE(style.props, combo, spacing, lua_nk_style_type_vec2);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "edit";
    /* edit background */
    PUSH_STYLE(style.props, combo, active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, edit, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, edit, hover, lua_nk_style_type_item);
    PUSH_STYLE(style.props, edit, normal, lua_nk_style_type_item);
    /* edit scrollbar */
    {
        PUSH_STYLE(style.props, edit.scrollbar, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, edit.scrollbar, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, edit.scrollbar, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, edit.scrollbar, normal, lua_nk_style_type_item);

        /* cursor */
        PUSH_STYLE(style.props, edit.scrollbar, cursor_active, lua_nk_style_type_cursor);
        PUSH_STYLE(style.props, edit.scrollbar, cursor_border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, edit.scrollbar, cursor_hover, lua_nk_style_type_cursor);
        PUSH_STYLE(style.props, edit.scrollbar, cursor_normal, lua_nk_style_type_cursor);

        /* properties */
        PUSH_STYLE(style.props, edit.scrollbar, border_cursor, lua_nk_style_type_float);
        PUSH_STYLE(style.props, edit.scrollbar, rounding_cursor, lua_nk_style_type_float);
        PUSH_STYLE(style.props, edit.scrollbar, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, edit.scrollbar, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, edit.scrollbar, rounding, lua_nk_style_type_float);

        /* optional buttons */
        PUSH_STYLE(style.props, edit.scrollbar, show_buttons, lua_nk_style_type_integer);
        {
            /* button background */
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, active, lua_nk_style_type_item);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, border_color, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, hover, lua_nk_style_type_item);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, normal, lua_nk_style_type_item);
            /* button text */
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, text_active, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, text_alignment, lua_nk_style_type_flags);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, text_background, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, text_hover, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, text_normal, lua_nk_style_type_color);
            /* button properties */
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, border, lua_nk_style_type_float);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, image_padding, lua_nk_style_type_vec2);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, padding, lua_nk_style_type_vec2);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, rounding, lua_nk_style_type_float);
            PUSH_STYLE(style.props, edit.scrollbar.dec_button, touch_padding, lua_nk_style_type_vec2);
        }
        PUSH_STYLE(style.props, edit.scrollbar, dec_symbol, lua_nk_style_type_symbol);
        {
            /* button background */
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, active, lua_nk_style_type_item);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, border_color, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, hover, lua_nk_style_type_item);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, normal, lua_nk_style_type_item);
            /* button text */
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, text_active, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, text_alignment, lua_nk_style_type_flags);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, text_background, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, text_hover, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, text_normal, lua_nk_style_type_color);
            /* button properties */
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, border, lua_nk_style_type_float);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, image_padding, lua_nk_style_type_vec2);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, padding, lua_nk_style_type_vec2);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, rounding, lua_nk_style_type_float);
            PUSH_STYLE(style.props, edit.scrollbar.inc_button, touch_padding, lua_nk_style_type_vec2);
        }
        PUSH_STYLE(style.props, edit.scrollbar, inc_symbol, lua_nk_style_type_symbol);
    }
    /* edit cursor */
    PUSH_STYLE(style.props, edit, cursor_hover, lua_nk_style_type_cursor);
    PUSH_STYLE(style.props, edit, cursor_normal, lua_nk_style_type_cursor);
    PUSH_STYLE(style.props, edit, cursor_text_hover, lua_nk_style_type_cursor);
    PUSH_STYLE(style.props, edit, cursor_text_normal, lua_nk_style_type_cursor);
    /* edit text (unselected) */
    PUSH_STYLE(style.props, edit, text_active, lua_nk_style_type_color);
    PUSH_STYLE(style.props, edit, text_hover, lua_nk_style_type_color);
    PUSH_STYLE(style.props, edit, text_normal, lua_nk_style_type_color);
    /* edit text (selected) */
    PUSH_STYLE(style.props, edit, selected_hover, lua_nk_style_type_color);
    PUSH_STYLE(style.props, edit, selected_normal, lua_nk_style_type_color);
    PUSH_STYLE(style.props, edit, selected_text_hover, lua_nk_style_type_color);
    PUSH_STYLE(style.props, edit, selected_text_normal, lua_nk_style_type_color);
    /* edit properties */
    PUSH_STYLE(style.props, edit, cursor_size, lua_nk_style_type_float);
    PUSH_STYLE(style.props, edit, border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, edit, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, edit, rounding, lua_nk_style_type_float);
    PUSH_STYLE(style.props, edit, row_padding, lua_nk_style_type_float);
    PUSH_STYLE(style.props, edit, scrollbar_size, lua_nk_style_type_vec2);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "progress";
    /* background */
    PUSH_STYLE(style.props, progress, active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, progress, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, progress, hover, lua_nk_style_type_item);
    PUSH_STYLE(style.props, progress, normal, lua_nk_style_type_item);

    /* cursor */
    PUSH_STYLE(style.props, progress, cursor_active, lua_nk_style_type_cursor);
    PUSH_STYLE(style.props, progress, cursor_border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, progress, cursor_hover, lua_nk_style_type_cursor);
    PUSH_STYLE(style.props, progress, cursor_normal, lua_nk_style_type_cursor);

    /* properties */
    PUSH_STYLE(style.props, progress, cursor_border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, progress, cursor_rounding, lua_nk_style_type_float);
    PUSH_STYLE(style.props, progress, border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, progress, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, progress, rounding, lua_nk_style_type_float);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "property";
    /* background */
    PUSH_STYLE(style.props, property, active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, property, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, property, hover, lua_nk_style_type_item);
    PUSH_STYLE(style.props, property, normal, lua_nk_style_type_item);

    /* text */
    PUSH_STYLE(style.props, property, label_active, lua_nk_style_type_color);
    PUSH_STYLE(style.props, property, label_normal, lua_nk_style_type_color);
    PUSH_STYLE(style.props, property, label_hover, lua_nk_style_type_color);

    /* symbols */
    PUSH_STYLE(style.props, property, sym_left, lua_nk_style_type_symbol);
    PUSH_STYLE(style.props, property, sym_right, lua_nk_style_type_symbol);

    /* properties */
    PUSH_STYLE(style.props, property, border, lua_nk_style_type_float);
    /* dec_button */
    {
        /* button background */
        PUSH_STYLE(style.props, property.dec_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, property.dec_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, property.dec_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, property.dec_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, property.dec_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, property.dec_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, property.dec_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, property.dec_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, property.dec_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, property.dec_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, property.dec_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, property.dec_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, property.dec_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, property.dec_button, touch_padding, lua_nk_style_type_vec2);
    }
    PUSH_STYLE(style.props, property, edit, lua_nk_style_type_edit);
    /* inc_button */
    {
        /* button background */
        PUSH_STYLE(style.props, property.inc_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, property.inc_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, property.inc_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, property.inc_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, property.inc_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, property.inc_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, property.inc_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, property.inc_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, property.inc_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, property.inc_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, property.inc_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, property.inc_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, property.inc_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, property.inc_button, touch_padding, lua_nk_style_type_vec2);
    }
    PUSH_STYLE(style.props, property, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, property, rounding, lua_nk_style_type_float);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "selectable";
    /* background (inactive) */
    PUSH_STYLE(style.props, selectable, hover, lua_nk_style_type_item);
    PUSH_STYLE(style.props, selectable, normal, lua_nk_style_type_item);
    PUSH_STYLE(style.props, selectable, pressed, lua_nk_style_type_item);

    /* background (active) */
    PUSH_STYLE(style.props, selectable, hover_active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, selectable, normal_active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, selectable, pressed_active, lua_nk_style_type_item);

    /* text (inactive) */
    PUSH_STYLE(style.props, selectable, text_normal, lua_nk_style_type_color);
    PUSH_STYLE(style.props, selectable, text_hover, lua_nk_style_type_color);
    PUSH_STYLE(style.props, selectable, text_pressed, lua_nk_style_type_color);

    /* text (active) */
    PUSH_STYLE(style.props, selectable, text_alignment, lua_nk_style_type_flags);
    PUSH_STYLE(style.props, selectable, text_background, lua_nk_style_type_color);
    PUSH_STYLE(style.props, selectable, text_normal_active, lua_nk_style_type_color);
    PUSH_STYLE(style.props, selectable, text_hover_active, lua_nk_style_type_color);
    PUSH_STYLE(style.props, selectable, text_pressed_active, lua_nk_style_type_color);

    /* properties */
    PUSH_STYLE(style.props, selectable, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, selectable, rounding, lua_nk_style_type_float);
    PUSH_STYLE(style.props, selectable, image_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, selectable, touch_padding, lua_nk_style_type_vec2);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "slider";
    PUSH_STYLE(style.props, slider, active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, slider, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, slider, hover, lua_nk_style_type_item);
    PUSH_STYLE(style.props, slider, normal, lua_nk_style_type_item);

    /* background bar */
    PUSH_STYLE(style.props, slider, bar_active, lua_nk_style_type_color);
    PUSH_STYLE(style.props, slider, bar_filled, lua_nk_style_type_color);
    PUSH_STYLE(style.props, slider, bar_hover, lua_nk_style_type_color);
    PUSH_STYLE(style.props, slider, bar_normal, lua_nk_style_type_color);

    /* cursor */
    PUSH_STYLE(style.props, slider, cursor_active, lua_nk_style_type_cursor);
    PUSH_STYLE(style.props, slider, cursor_hover, lua_nk_style_type_cursor);
    PUSH_STYLE(style.props, slider, cursor_normal, lua_nk_style_type_cursor);

    /* symbol */
    PUSH_STYLE(style.props, slider, dec_symbol, lua_nk_style_type_symbol);
    PUSH_STYLE(style.props, slider, inc_symbol, lua_nk_style_type_symbol);

    /* dec_button */
    {
        /* button background */
        PUSH_STYLE(style.props, slider.dec_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, slider.dec_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider.dec_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, slider.dec_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, slider.dec_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider.dec_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, slider.dec_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider.dec_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider.dec_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, slider.dec_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, slider.dec_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, slider.dec_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, slider.dec_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, slider.dec_button, touch_padding, lua_nk_style_type_vec2);
    }
    /* inc_button */
    {
        /* button background */
        PUSH_STYLE(style.props, slider.inc_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, slider.inc_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider.inc_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, slider.inc_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, slider.inc_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider.inc_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, slider.inc_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider.inc_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider.inc_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, slider.inc_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, slider.inc_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, slider.inc_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, slider.inc_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, slider.inc_button, touch_padding, lua_nk_style_type_vec2);
    }

    /* properties */
    PUSH_STYLE(style.props, slider, bar_height, lua_nk_style_type_float);
    PUSH_STYLE(style.props, slider, border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, slider, cursor_size, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, slider, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, slider, spacing, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, slider, show_buttons, lua_nk_style_type_integer);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "tab";
    /* background */
    PUSH_STYLE(style.props, tab, background, lua_nk_style_type_item);
    PUSH_STYLE(style.props, tab, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, tab, text, lua_nk_style_type_color);
    /* node_maximize_button */
    {
        /* button background */
        PUSH_STYLE(style.props, tab.node_maximize_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab.node_maximize_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.node_maximize_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab.node_maximize_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, tab.node_maximize_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.node_maximize_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, tab.node_maximize_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.node_maximize_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.node_maximize_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, tab.node_maximize_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab.node_maximize_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab.node_maximize_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab.node_maximize_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab.node_maximize_button, touch_padding, lua_nk_style_type_vec2);
    }
    /* node_minimize_button */
    {
        /* button background */
        PUSH_STYLE(style.props, tab.node_minimize_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab.node_minimize_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.node_minimize_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab.node_minimize_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, tab.node_minimize_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.node_minimize_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, tab.node_minimize_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.node_minimize_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.node_minimize_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, tab.node_minimize_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab.node_minimize_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab.node_minimize_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab.node_minimize_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab.node_minimize_button, touch_padding, lua_nk_style_type_vec2);
    }
    /* tab_maximize_button */
    {
        /* button background */
        PUSH_STYLE(style.props, tab.tab_maximize_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab.tab_maximize_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.tab_maximize_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab.tab_maximize_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, tab.tab_maximize_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.tab_maximize_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, tab.tab_maximize_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.tab_maximize_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.tab_maximize_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, tab.tab_maximize_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab.tab_maximize_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab.tab_maximize_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab.tab_maximize_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab.tab_maximize_button, touch_padding, lua_nk_style_type_vec2);
    }
    /* tab_minimize_button */
    {
        /* button background */
        PUSH_STYLE(style.props, tab.tab_minimize_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab.tab_minimize_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.tab_minimize_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab.tab_minimize_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, tab.tab_minimize_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.tab_minimize_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, tab.tab_minimize_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.tab_minimize_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab.tab_minimize_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, tab.tab_minimize_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab.tab_minimize_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab.tab_minimize_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab.tab_minimize_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab.tab_minimize_button, touch_padding, lua_nk_style_type_vec2);
    }
    /* symbol */
    PUSH_STYLE(style.props, tab, sym_maximize, lua_nk_style_type_symbol);
    PUSH_STYLE(style.props, tab, sym_minimize, lua_nk_style_type_symbol);

    /* properties */
    PUSH_STYLE(style.props, tab, border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, tab, indent, lua_nk_style_type_float);
    PUSH_STYLE(style.props, tab, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, tab, spacing, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, tab, rounding, lua_nk_style_type_float);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "window.header";
    /* background */
    PUSH_STYLE(style.props, window.header, active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, window.header, normal, lua_nk_style_type_item);
    PUSH_STYLE(style.props, window.header, hover, lua_nk_style_type_item);
    /* close_button */
    {
        /* button background */
        PUSH_STYLE(style.props, window.header.close_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, window.header.close_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header.close_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, window.header.close_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, window.header.close_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header.close_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, window.header.close_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header.close_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header.close_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, window.header.close_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, window.header.close_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, window.header.close_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, window.header.close_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, window.header.close_button, touch_padding, lua_nk_style_type_vec2);
    }
    /* minimize_button */
    {
        /* button background */
        PUSH_STYLE(style.props, window.header.minimize_button, active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, window.header.minimize_button, border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header.minimize_button, hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, window.header.minimize_button, normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, window.header.minimize_button, text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header.minimize_button, text_alignment, lua_nk_style_type_flags);
        PUSH_STYLE(style.props, window.header.minimize_button, text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header.minimize_button, text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header.minimize_button, text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, window.header.minimize_button, border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, window.header.minimize_button, image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, window.header.minimize_button, padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, window.header.minimize_button, rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, window.header.minimize_button, touch_padding, lua_nk_style_type_vec2);
    }
    /* symbol */
    PUSH_STYLE(style.props, window.header, close_symbol, lua_nk_style_type_symbol);
    PUSH_STYLE(style.props, window.header, minimize_symbol, lua_nk_style_type_symbol);
    PUSH_STYLE(style.props, window.header, maximize_symbol, lua_nk_style_type_symbol);

    /* title */
    PUSH_STYLE(style.props, window.header, label_active, lua_nk_style_type_color);
    PUSH_STYLE(style.props, window.header, label_hover, lua_nk_style_type_color);
    PUSH_STYLE(style.props, window.header, label_normal, lua_nk_style_type_color);

    /* properties */
    PUSH_STYLE(style.props, window.header, align, lua_nk_style_type_align);
    PUSH_STYLE(style.props, window.header, label_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window.header, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window.header, spacing, lua_nk_style_type_vec2);
    lua_nk_styles.push_back(style);
    style.props.clear();

    style.component = "window";
    /* window background */
    PUSH_STYLE(style.props, window, fixed_background, lua_nk_style_type_item);
    PUSH_STYLE(style.props, window, background, lua_nk_style_type_color);
    /* window colors */
    PUSH_STYLE(style.props, window, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, window, combo_border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, window, contextual_border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, window, group_border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, window, menu_border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, window, popup_border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, window, tooltip_border_color, lua_nk_style_type_color);
    /* window border */
    PUSH_STYLE(style.props, window, border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, window, combo_border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, window, contextual_border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, window, group_border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, window, menu_border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, window, popup_border, lua_nk_style_type_float);
    PUSH_STYLE(style.props, window, tooltip_border, lua_nk_style_type_float);
    /* window padding */
    PUSH_STYLE(style.props, window, combo_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window, contextual_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window, group_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window, menu_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window, min_row_height_padding, lua_nk_style_type_float);
    PUSH_STYLE(style.props, window, popup_padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window, tooltip_padding, lua_nk_style_type_vec2);
    /* window properties */
    PUSH_STYLE(style.props, window, min_size, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window, padding, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window, scrollbar_size, lua_nk_style_type_vec2);
    PUSH_STYLE(style.props, window, spacing, lua_nk_style_type_vec2);
    lua_nk_styles.push_back(style);

    wins[WINDOW_MainMenu].tmpl = "mainmenu";
    wins[WINDOW_MainMenu_Load].tmpl = "mainmenu_load";

    return nuklear;
}

bool Nuklear::Create(WindowType winType) {
    if (!lua)
        LuaInit();

    return LuaLoadTemplate(winType);
}

int Nuklear::KeyEvent(Io::GameKey key) {
    for (auto it = hotkeys.begin(); it < hotkeys.end(); it++) {
        struct hotkey hk = *it;
        if (hk.key == key) {
            lua_rawgeti(lua, LUA_REGISTRYINDEX, hk.callback);
            lua_pcall(lua, 0, 0, 0);

            return 1;
        }
    }

    return 0;
}

int lua_handle_error(lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    luaL_traceback(L, L, msg, 2);
    lua_remove(L, -2);

    return 1;
}

bool lua_error_check(WindowType winType, lua_State *L, int err) {
    if (err != 0) {
        logger->Warning("Nuklear: [%s] LUA error: %s\n", wins[winType].tmpl, lua_tostring(L, -1));
        lua_pop(L, 1);
        return true;
    }

    return false;
}

void Nuklear::Release(WindowType winType, bool is_reload) {
    nk_clear(ctx);

    if (wins[winType].tmpl && (wins[winType].state == WINDOW_INITIALIZED || wins[winType].state == WINDOW_TEMPLATE_ERROR)) {
        lua_getfield(lua, LUA_GLOBALSINDEX, "ui_release");
        currentWin = winType;
        int err = lua_pcall(lua, 0, 0, 0);
        if (lua_error_check(winType, lua, err)) {
            wins[winType].state = WINDOW_TEMPLATE_ERROR;
        }
        currentWin = WINDOW_null;

        int i = 0;
        for (auto it = wins[winType].img.begin(); it != wins[winType].img.end(); it++, i++) {
            if ((*it)->asset) {
                render->NuklearImageFree((*it)->asset);
                if ((*it)->asset->Release())
                    logger->Info("Nuklear: [%s] asset %d unloaded", wins[winType].tmpl, i);
                else
                    logger->Warning("Nuklear: [%s] asset %d unloading failed!", wins[winType].tmpl, i);

                delete *it;
            }
        }
        wins[winType].img.clear();
        wins[winType].img.swap(wins[winType].img);

        if (!is_reload && (wins[winType].state == WINDOW_INITIALIZED || wins[winType].state == WINDOW_TEMPLATE_ERROR))
            wins[winType].state = WINDOW_NOT_LOADED;

        logger->Info("Nuklear: [%s] template unloaded", wins[winType].tmpl);
    } else {
        logger->Warning("Nuklear: [%s] template is not loaded", wins[winType].tmpl);
    }
}

void Nuklear::Release(WindowType winType) {
    Release(winType, false);
}

bool Nuklear::Reload() {
    bool ret = true;

    for (int w = WINDOW_MainMenu; w != WINDOW_DebugMenu; w++) {
        if (wins[w].state == WINDOW_INITIALIZED || wins[w].state == WINDOW_TEMPLATE_ERROR) {
            Release((WindowType)w, true);
        }
    }

    for (auto itk = hotkeys.begin(); itk < hotkeys.end(); itk++) {
        struct hotkey hk = *itk;
        luaL_unref(lua, LUA_REGISTRYINDEX, hk.callback);
        logger->Info("Nuklear: hotkey '%s' is unset", GetDisplayName(hk.key).c_str());
    }
    hotkeys.clear();
    hotkeys.swap(hotkeys);

    LuaRelease();
    LuaInit();

    for (int w = WINDOW_MainMenu; w != WINDOW_DebugMenu; w++) {
        if (wins[w].state == WINDOW_INITIALIZED || wins[w].state == WINDOW_TEMPLATE_ERROR) {
            if (!Create(WindowType(w)))
                ret = false;
        }
    }

    return ret;
}

void Nuklear::Destroy() {
    render->NuklearRelease();
    for (auto it = lua_nk_styles.begin(); it < lua_nk_styles.end(); it++) {
        it->props.clear();
    }
    for (auto itk = hotkeys.begin(); itk < hotkeys.end(); itk++) {
        struct hotkey hk = *itk;
        luaL_unref(lua, LUA_REGISTRYINDEX, hk.callback);
        logger->Info("Nuklear: hotkey '%s' is unset", GetDisplayName(hk.key).c_str());
    }
    hotkeys.clear();
    lua_nk_styles.clear();
    free(ctx);
}

Nuklear::NUKLEAR_MODE Nuklear::Mode(WindowType winType) {
    return wins[winType].mode;
}

static void lua_dumptable(lua_State *L, int i) {
    lua_pushvalue(L, i);
    lua_pushnil(L);
    while (lua_next(L, 1)) {
        lua_pushvalue(L, 1);
        printf("%d\t%s\t", i, luaL_typename(L, -2));
        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                printf("%g\n", lua_tonumber(L, -2));
                break;
            case LUA_TSTRING:
                printf("%s\n", lua_tostring(L, -2));
                break;
            case LUA_TBOOLEAN:
                printf("%s\n", (lua_toboolean(L, -2) ? "true" : "false"));
                break;
            case LUA_TNIL:
                printf("%s\n", "nil");
                break;
            case LUA_TTABLE:
                lua_dumptable(L, -2);
                break;
            default:
                printf("%p\n", lua_topointer(L, -2));
                break;
        }
        lua_pop(L, 2);
    }
}

static void lua_dumpstack(lua_State *L) {
    int i;
    int top = lua_gettop(L);

    for (i = 0; i <= top; i++) {
        int t = lua_type(L, i);
        printf("%d\t%s\t", i, luaL_typename(L, i));
        switch (t) {
            case LUA_TSTRING:
                printf("%s\n", lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                printf("%s\n", lua_toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:
                printf("%g\n", lua_tonumber(L, i));
                break;
            case LUA_TNIL:
                printf("null\n");
                break;
            default:
                printf("%p\n", lua_topointer(L, i));
                break;
        }
    }
    printf("total: %d\n", top);
}

bool Nuklear::Draw(NUKLEAR_STAGE stage, WindowType winType, int id) {
    if (!lua)
        return false;

    if (wins[winType].state == WINDOW_INITIALIZED) {
        lua_getfield(lua, LUA_GLOBALSINDEX, "ui_draw");
        currentWin = winType;
        lua_pushinteger(lua, stage);
        int err = lua_pcall(lua, 1, 0, 0);
        if (lua_error_check(winType, lua, err)) {
            wins[winType].state = WINDOW_TEMPLATE_ERROR;
        }

        currentWin = WINDOW_null;
        if (stage == NUKLEAR_STAGE_POST) {
            nk_input_end(ctx);
            //render->BeginScene();
            render->NuklearRender(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
            //render->EndScene();
            //render->Present();
            nk_input_begin(ctx);
        }
    }

    return true;
}

void Nuklear::LuaRelease() {
    lua_close(lua);
}

bool Nuklear::LuaLoadTemplate(WindowType winType) {
    std::string name;

    if (!wins[winType].tmpl) {
        logger->Warning("Nuklear: [%s] unknown template", ToString(winType));
        return false;
    }

    name = wins[winType].tmpl;
    currentWin = winType;
    int status = luaL_loadfile(lua, MakeDataPath("UI", name + ".lua").c_str());
    if (status) {
        wins[winType].state = WINDOW_TEMPLATE_ERROR;
        logger->Warning("Nuklear: [%s] couldn't load lua template: %s", wins[winType].tmpl, lua_tostring(lua, -1));
        lua_pop(lua, 1);
        return false;
    }

    int err = lua_pcall(lua, 0, 0, 0);
    if (lua_error_check(winType, lua, err)) {
        wins[winType].state = WINDOW_TEMPLATE_ERROR;
        logger->Warning("Nuklear: [%s] error loading template: %s", wins[winType].tmpl, lua_tostring(lua, -1));
        return false;
    }
    currentWin = WINDOW_null;

    assert(lua_gettop(lua) == 0);
    wins[winType].mode = NUKLEAR_MODE(luaL_checkinteger(lua, 0));
    wins[winType].state = WINDOW_INITIALIZED;

    logger->Info("Nuklear: lua template '%s' loaded", name.c_str());

    return true;
}

static int lua_log_info(lua_State *L) {
    assert(lua_gettop(L) >= 1);
    const char *str = lua_tostring(lua, 1);
    logger->Info("Nuklear LUA: %s", str);

    return 0;
}

static int lua_log_warning(lua_State *L) {
    assert(lua_gettop(L) >= 1);
    const char *str = lua_tostring(lua, 1);
    logger->Warning("Nuklear LUA: %s", str);

    return 0;
}

#define lua_foreach(_lua, _idx) for (lua_pushnil(_lua); lua_next(_lua, _idx); lua_pop(_lua, 1))
#define lua_check_ret(_func) { int _ret = _func; if (_ret) return _ret; }


static int lua_nk_parse_vec2(lua_State *L, int idx, struct nk_vec2 *vec) {
    if (lua_istable(L, idx)) {
        lua_pushvalue(L, idx);
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            lua_pushvalue(L, -2);
            int i = luaL_checknumber(L, -1);
            switch (i) {
                case(1):
                    vec->x = luaL_checknumber(L, -2);
                    break;
                case(2):
                    vec->y = luaL_checknumber(L, -2);
                    break;
            }
            lua_pop(L, 2);
        }

        lua_pop(L, 1);
    } else {
        const char *msg = lua_pushfstring(L, "vec2 format is wrong");
        return luaL_argerror(L, -1, msg);
    }

    return 0;
}

static int lua_nk_is_hex(char c)
{
    return (c >= '0' && c <= '9')
        || (c >= 'a' && c <= 'f')
        || (c >= 'A' && c <= 'F');
}

static int lua_nk_parse_ratio(lua_State *L, int idx, std::vector<float> *ratio) {
    float floats[MAX_RATIO_ELEMENTS] = { 0 };

    if (lua_istable(L, idx)) {
        lua_pushvalue(L, idx);
        lua_pushnil(L);
        int count = 0;
        while (lua_next(L, -2)) {
            ratio->push_back(luaL_checknumber(L, -1));
            count++;

            lua_pop(L, 1);
        }

        lua_pop(L, 1);
    } else {
        const char *msg = lua_pushfstring(L, "ratio format is wrong");
        return luaL_argerror(L, -1, msg);
    }

    return 0;
}

static int lua_nk_parse_rect(lua_State *L, int idx, struct nk_rect *rect) {
    if (lua_istable(L, idx)) {
        lua_pushvalue(L, idx);
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            lua_pushvalue(L, -2);
            int i = lua_tonumber(L, -1);
            switch (i) {
                case(1):
                    rect->x = lua_tonumber(L, -2);
                    break;
                case(2):
                    rect->y = lua_tonumber(L, -2);
                    break;
                case(3):
                    rect->w = lua_tonumber(L, -2);
                    break;
                case(4):
                    rect->h = lua_tonumber(L, -2);
                    break;
            }
            lua_pop(L, 2);
        }

        lua_pop(L, 1);
    } else {
        const char *msg = lua_pushfstring(L, "rect format is wrong");
        return luaL_argerror(L, -1, msg);
    }

    return 0;
}

static int lua_nk_parse_color(lua_State *L, int idx, nk_color *color) {
    if (lua_istable(L, idx)) {
        lua_pushvalue(L, idx);
        lua_pushnil(L);
        color->a = 255;
        while (lua_next(L, -2)) {
            lua_pushvalue(L, -2);
            int i = lua_tonumber(L, -1);
            switch (i) {
                case(1):
                    color->r = lua_tonumber(L, -2);
                    break;
                case(2):
                    color->g = lua_tonumber(L, -2);
                    break;
                case(3):
                    color->b = lua_tonumber(L, -2);
                    break;
                case(4):
                    color->a = lua_tonumber(L, -2);
                    break;
            }
            lua_pop(L, 2);
        }

        lua_pop(L, 1);
        return 0;
    } else if (lua_isstring(L, idx)) {
        bool is_hex_color = true;
        const char *strcolor = luaL_checkstring(L, idx);
        size_t len = strlen(strcolor);
        if ((len == 7 || len == 9) && strcolor[0] == '#') {
            for (int i = 1; i < len; ++i) {
                if (!lua_nk_is_hex(strcolor[i])) {
                    is_hex_color = false;
                    break;
                }
            }

            if (is_hex_color) {
                int r, g, b, a = 255;
                sscanf(strcolor, "#%02x%02x%02x", &r, &g, &b);

                if (len == 9)
                    sscanf(strcolor + 7, "%02x", &a);

                color->r = r;
                color->g = g;
                color->b = b;
                color->a = a;
                return 0;
            }
        }
    }

    const char *msg = lua_pushfstring(L, "unrecognized color format");
    return luaL_argerror(L, -1, msg);
}

static int lua_nk_parse_image(lua_State *L, int idx, struct nk_image **image) {
    int slot = luaL_checknumber(L, idx);
    if (slot >= 0 && slot < wins[currentWin].img.size() && wins[currentWin].img[slot]->asset) {
        *image = &wins[currentWin].img[slot]->nk;
        return 0;
    }

    const char *msg = lua_pushfstring(L, "asset is wrong");
    return luaL_argerror(L, -1, msg);
}

static int lua_nk_parse_image_asset(lua_State *L, int idx, struct Image **asset) {
    int slot = luaL_checknumber(L, idx);
    if (slot >= 0 && slot < wins[currentWin].img.size() && wins[currentWin].img[slot]->asset) {
        *asset = wins[currentWin].img[slot]->asset;
        return 0;
    }

    const char *msg = lua_pushfstring(L, "asset is wrong");
    return luaL_argerror(L, -1, msg);
}

static int lua_nk_parse_style(lua_State *L, int cidx, int pidx, lua_nk_style_type **type, void **ptr) {
    const char *component = luaL_checkstring(L, cidx);
    const char *property = luaL_checkstring(L, pidx);

    for (auto it = lua_nk_styles.begin(); it < lua_nk_styles.end(); it++) {
        struct lua_nk_style style = *it;
        // logger->Info("component: %s", style.component);
        if (!strcmp(component, style.component)) {
            for (auto itp = style.props.begin(); itp < style.props.end(); itp++) {
                struct lua_nk_property prop = *itp;
                // logger->Info("property: %s", prop.property);
                if (!strcmp(property, prop.property)) {
                    *type = &prop.type;
                    *ptr = prop.ptr;

                    return 0;
                }
            }

            const char *msg = lua_pushfstring(L, "unknown property '%s'", property);
            return luaL_argerror(L, -1, msg);
        }
    }

    const char *msg = lua_pushfstring(L, "unknown component '%s", component);
    return luaL_argerror(L, -1, msg);
}

static int lua_nk_parse_style_button(lua_State *L, int idx, nk_style_button *style) {
    if (lua_istable(L, idx)) {
        lua_foreach(L, idx) {
            const char *key = luaL_checkstring(L, -2);
            if (!strcmp(key, "border")) {
                style->border = luaL_checknumber(L, -1);
            } else if (!strcmp(key, "border_color")) {
                lua_check_ret(lua_nk_parse_color(L, -1, &style->border_color));
            } else if (!strcmp(key, "hover")) {
                struct nk_image *img;
                lua_check_ret(lua_nk_parse_image(L, -1, &img));
                style->hover = nk_style_item_image(*img);
            } else if (!strcmp(key, "image_padding")) {
                lua_check_ret(lua_nk_parse_vec2(L, -1, &style->image_padding))
            } else if (!strcmp(key, "padding")) {
                lua_check_ret(lua_nk_parse_vec2(L, -1, &style->padding));
            } else if (!strcmp(key, "rounding")) {
                style->rounding = luaL_checknumber(L, -1);
            } else if (!strcmp(key, "touch_padding")) {
                lua_check_ret(lua_nk_parse_vec2(L, -1, &style->touch_padding))
            }
        }
    } else {
        const char *msg = lua_pushfstring(L, "wrong button style argument");
        return luaL_argerror(L, -1, msg);
    }

    return 0;
}

static int lua_nk_parse_window_flags(lua_State *L, int idx, nk_flags *flags) {
    *flags = NK_WINDOW_NO_SCROLLBAR;

    if (lua_istable(L, idx)) {
        lua_foreach(L, idx) {
            const char *flag = luaL_checkstring(L, -1);
            if (!strcmp(flag, "border"))
                *flags |= NK_WINDOW_BORDER;
            else if (!strcmp(flag, "movable"))
                *flags |= NK_WINDOW_MOVABLE;
            else if (!strcmp(flag, "scalable"))
                *flags |= NK_WINDOW_SCALABLE;
            else if (!strcmp(flag, "closable"))
                *flags |= NK_WINDOW_CLOSABLE;
            else if (!strcmp(flag, "minimizable"))
                *flags |= NK_WINDOW_MINIMIZABLE;
            else if (!strcmp(flag, "scrollbar"))
                *flags &= ~NK_WINDOW_NO_SCROLLBAR;
            else if (!strcmp(flag, "title"))
                *flags |= NK_WINDOW_TITLE;
            else if (!strcmp(flag, "scroll_auto_hide"))
                *flags |= NK_WINDOW_SCROLL_AUTO_HIDE;
            else if (!strcmp(flag, "background"))
                *flags |= NK_WINDOW_BACKGROUND;
            else if (!strcmp(flag, "scale_left"))
                *flags |= NK_WINDOW_SCALE_LEFT;
            else if (!strcmp(flag, "no_input"))
                *flags |= NK_WINDOW_NO_INPUT;
            else {
                const char *msg = lua_pushfstring(L, "unrecognized window flag '%s'", flag);
                return luaL_argerror(L, -1, msg);
            }
        }
    } else {
        const char *msg = lua_pushfstring(L, "wrong window flag argument");
        return luaL_argerror(L, -1, msg);
    }

    lua_pop(lua, 1);

    return 0;
}

static int lua_nk_parse_text_align(lua_State *L, int idx, nk_flags *flag) {
    const char *strflag = luaL_checkstring(L, idx);
    if (!strcmp(strflag, "left"))
        *flag = NK_TEXT_ALIGN_LEFT;
    else if (!strcmp(strflag, "center"))
        *flag = NK_TEXT_ALIGN_CENTERED;
    else if (!strcmp(strflag, "right"))
        *flag = NK_TEXT_ALIGN_RIGHT;

    return 0;
}

static int lua_nk_parse_symbol(lua_State *L, int idx, nk_symbol_type *symbol) {
    const char *strsymbol = luaL_checkstring(L, idx);
    if (!strcmp(strsymbol, "x"))
        *symbol = nk_symbol_type::NK_SYMBOL_X;
    else if (!strcmp(strsymbol, "underscore"))
        *symbol = nk_symbol_type::NK_SYMBOL_UNDERSCORE;
    else if (!strcmp(strsymbol, "circle_solid"))
        *symbol = nk_symbol_type::NK_SYMBOL_CIRCLE_SOLID;
    else if (!strcmp(strsymbol, "circle_outline"))
        *symbol = nk_symbol_type::NK_SYMBOL_CIRCLE_OUTLINE;
    else if (!strcmp(strsymbol, "rect_solid"))
        *symbol = nk_symbol_type::NK_SYMBOL_RECT_SOLID;
    else if (!strcmp(strsymbol, "rect_outline"))
        *symbol = nk_symbol_type::NK_SYMBOL_RECT_OUTLINE;
    else if (!strcmp(strsymbol, "triangle_up"))
        *symbol = nk_symbol_type::NK_SYMBOL_TRIANGLE_UP;
    else if (!strcmp(strsymbol, "triangle_down"))
        *symbol = nk_symbol_type::NK_SYMBOL_TRIANGLE_DOWN;
    else if (!strcmp(strsymbol, "triangle_left"))
        *symbol = nk_symbol_type::NK_SYMBOL_TRIANGLE_LEFT;
    else if (!strcmp(strsymbol, "triangle_right"))
        *symbol = nk_symbol_type::NK_SYMBOL_TRIANGLE_RIGHT;
    else if (!strcmp(strsymbol, "triangle_plus"))
        *symbol = nk_symbol_type::NK_SYMBOL_PLUS;
    else if (!strcmp(strsymbol, "triangle_minus"))
        *symbol = nk_symbol_type::NK_SYMBOL_MINUS;
    else if (!strcmp(strsymbol, "triangle_max"))
        *symbol = nk_symbol_type::NK_SYMBOL_MAX;

    return 0;
}

static int lua_nk_begin(lua_State *L) {
    assert(lua_gettop(L) >= 3 && lua_gettop(L) <= 4);

    const char *name, *title = NULL;
    nk_flags flags = 0;
    struct nk_rect rect;
    name = luaL_checkstring(L, 1);
    lua_check_ret(lua_nk_parse_rect(L, 2, &rect));
    lua_check_ret(lua_nk_parse_window_flags(L, 3, &flags));
    if (lua_gettop(L) == 4)
        title = luaL_checkstring(L, 4);

    bool ret = nk_begin_titled(nuklear->ctx, name, title ? title : name, rect, flags);
    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_button_color(lua_State *L) {
    assert(lua_gettop(L) == 1);

    struct nk_color color;
    lua_check_ret(lua_nk_parse_color(L, 1, &color));

    bool ret = nk_button_color(nuklear->ctx, color);
    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_button_image(lua_State *L) {
    assert(lua_gettop(L) >= 1 && lua_gettop(L) <= 4 && lua_gettop(L) != 2);

    struct nk_style_button style = {};
    bool styled = false;
    nk_flags flag = 0;
    int slot = luaL_checkinteger(L, 1);
    struct nk_image *img;
    const char *label = NULL;

    lua_check_ret(lua_nk_parse_image(L, 1, &img));

    if (lua_gettop(L) >= 3 && lua_isstring(L, 2)) {
        label = luaL_checkstring(L, 2);
        lua_check_ret(lua_nk_parse_text_align(L, 3, &flag));
    }

    if (lua_gettop(L) == 4) {
        lua_check_ret(lua_nk_parse_style_button(L, 4, &style));
        styled = true;
    }

    bool ret;
    if (label != NULL && styled)
        ret = nk_button_image_label_styled(nuklear->ctx, &style, *img, label, flag);
    else if (label != NULL)
        ret = nk_button_image_label(nuklear->ctx, *img, label, flag);
    else if (styled)
        ret = nk_button_image_styled(nuklear->ctx, &style, *img);
    else
        ret = nk_button_image(nuklear->ctx, *img);

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_button_label(lua_State *L) {
    assert(lua_gettop(L) >= 1 && lua_gettop(L) <= 2);

    struct nk_style_button style = {};
    const char *label = luaL_checkstring(L, 1);

    bool ret;
    if (lua_gettop(L) == 2) {
        lua_check_ret(lua_nk_parse_style_button(L, 2, &style));
        ret = nk_button_label_styled(nuklear->ctx, &style, label);
    } else {
        ret = nk_button_label(nuklear->ctx, label);
    }

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_button_symbol(lua_State *L) {
    assert(lua_gettop(L) >= 1 && lua_gettop(L) <= 4 && lua_gettop(L) != 2);
    struct nk_style_button style = {};
    const char *label = NULL;
    nk_flags flag = 0;
    bool styled = false;
    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;

    lua_check_ret(lua_nk_parse_symbol(L, 1, &symbol))

    if (lua_gettop(L) >= 3 && lua_isstring(L, 2)) {
        label = luaL_checkstring(L, 2);
        lua_check_ret(lua_nk_parse_text_align(L, 3, &flag));
    }

    if (lua_gettop(L) == 4) {
        lua_check_ret(lua_nk_parse_style_button(L, 4, &style));
        styled = true;
    }

    bool ret;
    if (label != NULL && styled)
        ret = nk_button_symbol_label_styled(nuklear->ctx, &style, symbol, label, flag);
    else if (label != NULL)
        ret = nk_button_symbol_label(nuklear->ctx, symbol, label, flag);
    else if (styled)
        ret = nk_button_symbol_styled(nuklear->ctx, &style, symbol);
    else
        ret = nk_button_symbol(nuklear->ctx, symbol);

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_checkbox(lua_State *L) {
    assert(lua_gettop(L) == 2);

    const char *label = luaL_checkstring(L, 1);
    nk_bool value = lua_toboolean(L, 2);

    bool ret = nk_checkbox_label(nuklear->ctx, label, &value);
    lua_pushboolean(L, value);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_combo_begin_color(lua_State *L) {
    assert(lua_gettop(L) == 1);

    struct nk_color color;
    struct nk_vec2 size;

    lua_check_ret(lua_nk_parse_color(L, 1, &color));
    lua_check_ret(lua_nk_parse_vec2(L, 2, &size));

    bool ret = nk_combo_begin_color(nuklear->ctx, color, size);
    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_combo_begin_image(lua_State *L) {
    assert(lua_gettop(L) >= 2 && lua_gettop(L) <= 3);

    struct nk_image *img;
    struct nk_vec2 size;
    const char *label = NULL;

    lua_check_ret(lua_nk_parse_image(L, 1, &img));
    lua_check_ret(lua_nk_parse_vec2(L, 2, &size));

    if (lua_gettop(L) == 3) {
        label = luaL_checkstring(L, 3);
    }

    bool ret;
    if (label)
        ret = nk_combo_begin_image_label(nuklear->ctx, label, *img, size);
    else
        ret = nk_combo_begin_image(nuklear->ctx, *img, size);

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_combo_begin_label(lua_State *L) {
    assert(lua_gettop(L) == 2);

    struct nk_vec2 size;
    const char *label = luaL_checkstring(L, 1);
    lua_check_ret(lua_nk_parse_vec2(L, 2, &size));

    bool ret = nk_combo_begin_label(nuklear->ctx, label, size);

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_combo_begin_symbol(lua_State *L) {
    assert(lua_gettop(L) >= 2 && lua_gettop(L) <= 3);

    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;
    struct nk_vec2 size;
    const char *label = NULL;

    lua_check_ret(lua_nk_parse_symbol(L, 1, &symbol))
    lua_check_ret(lua_nk_parse_vec2(L, 2, &size));
    if (lua_gettop(L) == 3) {
        label = luaL_checkstring(L, 3);
    }

    bool ret;
    if (label)
        ret = nk_combo_begin_symbol_label(nuklear->ctx, label, symbol, size);
    else
        ret = nk_combo_begin_symbol(nuklear->ctx, symbol, size);

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_combo_item_image(lua_State *L) {
    assert(lua_gettop(L) == 3);

    struct nk_image *img;
    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 2);
    lua_check_ret(lua_nk_parse_image(L, 1, &img));
    lua_check_ret(lua_nk_parse_text_align(L, 3, &flag));

    bool ret = nk_combo_item_image_label(nuklear->ctx, *img, label, flag);

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_image_dimensions(lua_State *L) {
    assert(lua_gettop(L) == 1);

    int slot = luaL_checkinteger(L, 1);
    struct Image *asset = NULL;

    lua_check_ret(lua_nk_parse_image_asset(L, 1, &asset));

    lua_pushnumber(L, asset->GetHeight());
    lua_pushnumber(L, asset->GetWidth());

    return 2;
}

static int lua_nk_combo_item_label(lua_State *L) {
    assert(lua_gettop(L) == 2);

    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 1);
    lua_check_ret(lua_nk_parse_text_align(L, 2, &flag));

    bool ret = nk_combo_item_label(nuklear->ctx, label, flag);

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_combo_item_symbol(lua_State *L) {
    assert(lua_gettop(L) == 3);

    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;
    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 2);
    lua_check_ret(lua_nk_parse_symbol(L, 1, &symbol))
        lua_check_ret(lua_nk_parse_text_align(L, 3, &flag));

    bool ret = nk_combo_item_symbol_label(nuklear->ctx, symbol, label, flag);

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_combo_end(lua_State *L) {
    assert(lua_gettop(L) == 0);

    nk_combo_end(nuklear->ctx);

    return 0;
}

static int lua_nk_end(lua_State *L) {
    assert(lua_gettop(L) == 0);

    nk_end(nuklear->ctx);

    return 0;
}

static int lua_nk_group_begin(lua_State *L) {
    assert(lua_gettop(L) == 2);

    const char *title = luaL_checkstring(L, 1);
    nk_flags flags;
    lua_check_ret(lua_nk_parse_window_flags(L, 2, &flags));

    bool ret = nk_group_begin(nuklear->ctx, title, flags);
    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_group_end(lua_State *L) {
    assert(lua_gettop(L) == 0);

    nk_group_end(nuklear->ctx);

    return 0;
}

static int lua_nk_load_font(lua_State *L) {
    assert(lua_gettop(L) == 2);

    const char *fontname = luaL_checkstring(L, 1);
    size_t fontsize = luaL_checknumber(L, 2);
    std::string fontpath = MakeDataPath("fonts", fontname);

    struct nk_font *font = render->NuklearLoadFont(fontpath.c_str(), fontsize);
    if (font)
        nk_style_set_font(nuklear->ctx, &font->handle);

    lua_pushboolean(L, !!font);

    return 1;
}

static int lua_nk_label(lua_State *L) {
    assert(lua_gettop(L) == 2);

    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 1);
    lua_check_ret(lua_nk_parse_text_align(L, 2, &flag));

    nk_label(nuklear->ctx, label, flag);

    return 0;
}

static int lua_nk_layout_row(lua_State *L) {
    assert(lua_gettop(L) == 4);

    const char *strfmt = luaL_checkstring(L, 1);
    float height = luaL_checknumber(L, 2);
    float cols = luaL_checknumber(L, 3);
    std::vector<float> ratio;
    lua_check_ret(lua_nk_parse_ratio(L, 4, &ratio));
    float *floats = (float *)calloc(ratio.size(), sizeof(float));

    int i = 0;
    for (auto it = ratio.begin(); it != ratio.end(); it++, i++) {
        floats[i] = *it;
    }
    ratio.clear();

    enum nk_layout_format fmt;

    if (!strcmp(strfmt, "dynamic"))
        fmt = NK_DYNAMIC;
    else if (!strcmp(strfmt, "static"))
        fmt = NK_STATIC;
    else {
        free(floats);
        const char *msg = lua_pushfstring(L, "unrecognized format flag '%s'", strfmt);
        return luaL_argerror(L, -1, msg);
    }

    nk_layout_row(nuklear->ctx, fmt, height, cols, floats);
    free(floats);

    return 0;
}

static int lua_nk_layout_row_dynamic(lua_State *L) {
    assert(lua_gettop(L) == 2);
    float height = luaL_checknumber(L, 1);
    float cols = luaL_checknumber(L, 2);

    nk_layout_row_dynamic(nuklear->ctx, height, cols);

    return 0;
}

static int lua_nk_layout_row_static(lua_State *L) {
    assert(lua_gettop(L) == 3);
    float height = luaL_checknumber(L, 1);
    float width = luaL_checknumber(L, 2);
    float cols = luaL_checknumber(L, 3);

    nk_layout_row_static(nuklear->ctx, height, width, cols);

    return 0;
}

static int lua_nk_layout_row_begin(lua_State *L) {
    assert(lua_gettop(L) == 3);
    const char *strfmt = luaL_checkstring(L, 1);
    float height = luaL_checknumber(L, 2);
    float cols = luaL_checknumber(L, 3);

    enum nk_layout_format fmt;

    if (!strcmp(strfmt, "dynamic"))
        fmt = NK_DYNAMIC;
    else
        fmt = NK_STATIC;

    nk_layout_row_begin(nuklear->ctx, fmt, height, cols);

    return 0;
}

static int lua_nk_layout_row_push(lua_State *L) {
    assert(lua_gettop(L) == 1);
    float ratio = luaL_checknumber(L, 1);

    nk_layout_row_push(nuklear->ctx, ratio);

    return 0;
}

static int lua_nk_layout_row_end(lua_State *L) {
    assert(lua_gettop(L) == 0);

    nk_layout_row_end(nuklear->ctx);

    return 0;
}

static int lua_nk_style_from_table(lua_State *L) {
    struct nk_color table[NK_COLOR_COUNT];
    struct nk_color color = { 0, 0, 0, 255 };

    assert(lua_gettop(L) == 1);
    if (lua_istable(L, 1)) {
        lua_foreach(L, 1) {
            bool is_hex_color = true;

            const char *key = luaL_checkstring(L, -2);
            lua_check_ret(lua_nk_parse_color(L, -1, &color));

            if (!strcmp(key, "border"))
                table[NK_COLOR_BORDER] = color;
            else if (!strcmp(key, "button"))
                table[NK_COLOR_BUTTON] = color;
            else if (!strcmp(key, "button_active"))
                table[NK_COLOR_BUTTON_ACTIVE] = color;
            else if (!strcmp(key, "button_hover"))
                table[NK_COLOR_BUTTON_HOVER] = color;
            else if (!strcmp(key, "chart"))
                table[NK_COLOR_CHART] = color;
            else if (!strcmp(key, "chart_color"))
                table[NK_COLOR_CHART_COLOR] = color;
            else if (!strcmp(key, "chart_color_highlight"))
                table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = color;
            else if (!strcmp(key, "combo"))
                table[NK_COLOR_COMBO] = color;
            else if (!strcmp(key, "count"))
                table[NK_COLOR_COUNT] = color;
            else if (!strcmp(key, "edit"))
                table[NK_COLOR_EDIT] = color;
            else if (!strcmp(key, "edit_cursor"))
                table[NK_COLOR_EDIT_CURSOR] = color;
            else if (!strcmp(key, "header"))
                table[NK_COLOR_HEADER] = color;
            else if (!strcmp(key, "property"))
                table[NK_COLOR_PROPERTY] = color;
            else if (!strcmp(key, "scrollbar"))
                table[NK_COLOR_SCROLLBAR] = color;
            else if (!strcmp(key, "scrollbar_cursor"))
                table[NK_COLOR_SCROLLBAR_CURSOR] = color;
            else if (!strcmp(key, "scrollbar_cursor_active"))
                table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = color;
            else if (!strcmp(key, "scrollbar_cursor_hover"))
                table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = color;
            else if (!strcmp(key, "select"))
                table[NK_COLOR_SELECT] = color;
            else if (!strcmp(key, "select_active"))
                table[NK_COLOR_SELECT_ACTIVE] = color;
            else if (!strcmp(key, "slider"))
                table[NK_COLOR_SLIDER] = color;
            else if (!strcmp(key, "slider_cursor"))
                table[NK_COLOR_SLIDER_CURSOR] = color;
            else if (!strcmp(key, "slider_cursor_active"))
                table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = color;
            else if (!strcmp(key, "slider_cursor_hover"))
                table[NK_COLOR_SLIDER_CURSOR_HOVER] = color;
            else if (!strcmp(key, "tab_header"))
                table[NK_COLOR_TAB_HEADER] = color;
            else if (!strcmp(key, "text"))
                table[NK_COLOR_TEXT] = color;
            else if (!strcmp(key, "toggle"))
                table[NK_COLOR_TOGGLE] = color;
            else if (!strcmp(key, "toggle_cursor"))
                table[NK_COLOR_TOGGLE_CURSOR] = color;
            else if (!strcmp(key, "toggle_hover"))
                table[NK_COLOR_TOGGLE_HOVER] = color;
            else if (!strcmp(key, "window"))
                table[NK_COLOR_WINDOW] = color;
        }
    }
    lua_pop(lua, 1);

    nk_style_from_table(nuklear->ctx, table);

    return 0;
}

static int lua_nk_style_get(lua_State *L) {
    assert(lua_gettop(L) == 2);
    lua_nk_style_type *type;
    void *ptr = nullptr;
    nk_color *color;
    float *value;
    struct nk_image im;
    struct nk_image *im2;
    struct nk_image im3;
    struct nk_vec2 *vec2;
    char *msg;
    int i = 0;

    lua_check_ret(lua_nk_parse_style(L, 1, 2, &type, &ptr));

    bool ret = false;
    switch (*type) {
        case(lua_nk_style_type_color):
            color = (nk_color *)ptr;

            lua_newtable(L);
            lua_pushliteral(L, "r");
            lua_pushnumber(L, color->r);
            lua_rawset(L, -3);
            lua_pushliteral(L, "g");
            lua_pushnumber(L, color->g);
            lua_rawset(L, -3);
            lua_pushliteral(L, "b");
            lua_pushnumber(L, color->b);
            lua_rawset(L, -3);
            lua_pushliteral(L, "a");
            lua_pushnumber(L, color->a);
            lua_rawset(L, -3);
            return 1;
        case(lua_nk_style_type_float):
            value = (float *)ptr;

            lua_pushnumber(L, *value);
            return 1;
        case(lua_nk_style_type_item):
            //nk_style_item_type
            break;
        case(lua_nk_style_type_vec2):
            vec2 = (struct nk_vec2 *)ptr;

            lua_newtable(L);
            lua_pushliteral(L, "x");
            lua_pushnumber(L, vec2->x);
            lua_rawset(L, -3);
            lua_pushliteral(L, "y");
            lua_pushnumber(L, vec2->y);
            lua_rawset(L, -3);
            return 1;
    }

    return luaL_argerror(L, -1, lua_pushfstring(L, "not implemented yet"));
}

static int lua_nk_style_pop(lua_State *L) {
    assert(lua_gettop(L) == 2);

    lua_nk_style_type *type;
    void *ptr = nullptr;
    lua_check_ret(lua_nk_parse_style(L, 1, 2, &type, &ptr));

    bool ret;
    switch (*type) {
        case(lua_nk_style_type_color):
            ret = nk_style_pop_color(nuklear->ctx);
            break;
        case(lua_nk_style_type_float):
            ret = nk_style_pop_float(nuklear->ctx);
            break;
        case(lua_nk_style_type_flags):
            ret = nk_style_pop_flags(nuklear->ctx);
            break;
        case(lua_nk_style_type_cursor):
        case(lua_nk_style_type_item):
        case(lua_nk_style_type_symbol):
            ret = nk_style_pop_style_item(nuklear->ctx);
            break;
        case(lua_nk_style_type_vec2):
            ret = nk_style_pop_vec2(nuklear->ctx);
            break;
    }

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_style_push(lua_State *L) {
    assert(lua_gettop(L) == 3);
    lua_nk_style_type *type;
    void *ptr = nullptr;
    nk_color color;
    float value;
    struct nk_image *img;
    struct nk_vec2 vec2;
    nk_style_item item;

    lua_check_ret(lua_nk_parse_style(L, 1, 2, &type, &ptr));

    bool ret;
    switch (*type) {
        case(lua_nk_style_type_color):
            lua_check_ret(lua_nk_parse_color(L, 3, &color));
            ret = nk_style_push_color(nuklear->ctx, (nk_color *)ptr, color);
            break;
        case(lua_nk_style_type_float):
            value = luaL_checknumber(L, 3);
            ret = nk_style_push_float(nuklear->ctx, (float *)ptr, value);
            break;
        case(lua_nk_style_type_item):
            if (lua_isnumber(L, 3)) {
                lua_check_ret(lua_nk_parse_image(L, 3, &img));
                ret = nk_style_push_style_item(nuklear->ctx, (nk_style_item *)ptr, nk_style_item_image(*img));
            } else if (lua_istable(L, 3) || lua_isstring(L, 3)) {
                lua_check_ret(lua_nk_parse_color(L, 3, &color));
                ret = nk_style_push_style_item(nuklear->ctx, (nk_style_item *)ptr, nk_style_item_color(color));
            } else {
                // TODO: nine slice
                return luaL_argerror(L, -1, lua_pushfstring(L, "not implemented yet"));
            }

            break;
        case(lua_nk_style_type_vec2):
            lua_check_ret(lua_nk_parse_vec2(L, 3, &vec2));
            ret = nk_style_push_vec2(nuklear->ctx, (struct nk_vec2 *)ptr, vec2);
            break;
    }

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_spacer(lua_State *L) {
    assert(lua_gettop(L) == 0);

    nk_spacer(nuklear->ctx);

    return 0;
}

static int lua_nk_load_image(lua_State *L) {
    assert(lua_gettop(L) >= 2);

    const char *type = luaL_checkstring(L, 1);
    const char *name = luaL_checkstring(L, 2);
    int mask = 0x7FF;
    if (lua_gettop(L) == 3)
        mask = luaL_checkinteger(L, 3);

    struct img *im = new(struct img);

    int slot = -1;
    bool ret = false;
    if (!strcmp(type, "pcx"))
        im->asset = assets->GetImage_PCXFromIconsLOD(name);
    else if (!strcmp(type, "bmp"))
        im->asset = assets->GetImage_ColorKey(name, mask);
    else
        goto finish;

    if (!im->asset)
        goto finish;

    im->nk = render->NuklearImageLoad(im->asset);

    slot = wins[currentWin].img.size();
    wins[currentWin].img.push_back(im);

    logger->Info("Nuklear: [%s] asset %d: '%s', type '%s' loaded!", wins[currentWin].tmpl, slot, name, type);

    lua_pushnumber(L, slot);
    return 1;

finish:
    delete im;
    logger->Warning("Nuklear: [%s] asset '%s', type '%s' loading failed!", wins[currentWin].tmpl, name, type);

    lua_pushnil(L);
    return 1;
}

static int lua_window_dimensions(lua_State *L) {
    assert(lua_gettop(L) == 0);

    lua_pushnumber(L, window->GetWidth());
    lua_pushnumber(L, window->GetHeight());
    lua_pushnumber(L, render->GetRenderWidth());
    lua_pushnumber(L, render->GetRenderHeight());

    return 4;
}

static int lua_set_game_current_menu(lua_State *L) {
    assert(lua_gettop(L) == 1);

    int id = luaL_checkinteger(L, 1);
    SetCurrentMenuID(MENU_STATE(id));

    return 4;
}

static int lua_set_hotkey(lua_State *L) {
    assert(lua_gettop(L) == 2);

    Io::GameKey gameKey;
    const char *key = luaL_checkstring(L, 1);
    if (!TryParseDisplayName(key, &gameKey)) {
        return luaL_argerror(L, -1, lua_pushfstring(L, "key '%s' is unknown", key));
    }

    if (lua_isfunction(L, 2)) {
        struct hotkey hk;
        hk.key = gameKey;
        hk.callback = luaL_ref(L, LUA_REGISTRYINDEX);

        for (auto itk = hotkeys.begin(); itk < hotkeys.end(); itk++) {
            struct hotkey ithk = *itk;
            if (ithk.key == hk.key) {
                luaL_unref(L, LUA_REGISTRYINDEX, ithk.callback);
                ithk.callback = hk.callback;
                logger->Info("Nuklear: hotkey '%s' is reset", key);

                return 0;
            }
        }

        hotkeys.push_back(hk);
        logger->Info("Nuklear: hotkey '%s' is set", key);

        return 0;
    }

    return luaL_argerror(L, -1, lua_pushfstring(L, "callback is wrong"));
}

static int lua_unset_hotkey(lua_State *L) {
    assert(lua_gettop(L) == 1);

    Io::GameKey gameKey;
    const char *key = luaL_checkstring(L, 1);
    if (!TryParseDisplayName(key, &gameKey)) {
        return luaL_argerror(L, -1, lua_pushfstring(L, "key '%s' is unknown", key));
    }

    int i = 0;
    for (auto itk = hotkeys.begin(); itk < hotkeys.end(); itk++, i++) {
        struct hotkey hk = *itk;
        if (gameKey == hk.key) {
            luaL_unref(lua, LUA_REGISTRYINDEX, hk.callback);
            logger->Info("Nuklear: hotkey '%s' is unset", key);
            hotkeys.erase(itk);
            return 0;
        }
    }

    return 0;
}

static int lua_unset_hotkeys(lua_State *L) {
    assert(lua_gettop(L) == 0);

    int i = 0;
    for (auto itk = hotkeys.begin(); itk < hotkeys.end(); itk++, i++) {
        struct hotkey hk = *itk;
        luaL_unref(lua, LUA_REGISTRYINDEX, hk.callback);
        logger->Info("Nuklear: hotkey '%s' is unset", GetDisplayName(hk.key).c_str());
    }
    hotkeys.clear();

    return 0;
}

bool Nuklear::LuaInit() {
    lua = luaL_newstate();

    lua_gc(lua, LUA_GCSTOP, 0);
    luaL_openlibs(lua);
    lua_gc(lua, LUA_GCRESTART, -1);

    lua_getglobal(lua, "package");
    lua_pushfstring(lua, MakeDataPath("UI", "?.lua").c_str());
    lua_setfield(lua, -2, "path");
    lua_pushstring(lua, "");
    lua_setfield(lua, -2, "cpath");
    lua_pop(lua, 1);

    static const luaL_Reg log[] =
    {
        { "info", lua_log_info },
        { "warning", lua_log_warning },
        { NULL, NULL }
    };

    luaL_newlib(lua, log);
    lua_setglobal(lua, "log");

    static const luaL_Reg ui[] =
    {
        { "nk_begin", lua_nk_begin },
        { "nk_button_color", lua_nk_button_color },
        { "nk_button_image", lua_nk_button_image },
        { "nk_button_label", lua_nk_button_label },
        { "nk_button_symbol", lua_nk_button_symbol },
        { "nk_checkbox", lua_nk_checkbox },
        { "nk_combo_begin_color", lua_nk_combo_begin_color },
        { "nk_combo_begin_image", lua_nk_combo_begin_image },
        { "nk_combo_begin_label", lua_nk_combo_begin_label },
        { "nk_combo_begin_symbol", lua_nk_combo_begin_symbol },
        { "nk_combo_item_image", lua_nk_combo_item_image },
        { "nk_combo_item_label", lua_nk_combo_item_label },
        { "nk_combo_item_symbol", lua_nk_combo_item_symbol },
        { "nk_combo_end", lua_nk_combo_end },
        { "nk_end", lua_nk_end },
        { "nk_group_begin", lua_nk_group_begin },
        { "nk_group_end", lua_nk_group_end },
        { "nk_image_dimensions", lua_nk_image_dimensions },
        { "nk_label", lua_nk_label },
        { "nk_load_font", lua_nk_load_font },
        { "nk_load_image", lua_nk_load_image },
        { "nk_layout_row", lua_nk_layout_row },
        { "nk_layout_row_begin", lua_nk_layout_row_begin },
        { "nk_layout_row_dynamic", lua_nk_layout_row_dynamic },
        { "nk_layout_row_push", lua_nk_layout_row_push },
        { "nk_layout_row_static", lua_nk_layout_row_static },
        { "nk_layout_row_end", lua_nk_layout_row_end },
        { "nk_spacer", lua_nk_spacer },
        { "nk_style_from_table", lua_nk_style_from_table },
        { "nk_style_get", lua_nk_style_get },
        { "nk_style_pop", lua_nk_style_pop },
        { "nk_style_push", lua_nk_style_push },
        { NULL, NULL }
    };
    luaL_newlib(lua, ui);
    lua_setglobal(lua, "ui");

    static const luaL_Reg game[] =
    {
        { "set_current_menu", lua_set_game_current_menu },
        { "set_hotkey", lua_set_hotkey },
        { "unset_hotkey", lua_unset_hotkey },
        { "unset_hotkeys", lua_unset_hotkeys },
        { NULL, NULL }
    };
    luaL_newlib(lua, game);
    lua_setglobal(lua, "game");

    static const luaL_Reg window[] =
    {
        { "dimensions", lua_window_dimensions },
        { NULL, NULL }
    };
    luaL_newlib(lua, window);
    lua_setglobal(lua, "window");

    lua_pushinteger(lua, NUKLEAR_STAGE_PRE);
    lua_setglobal(lua, "NUKLEAR_STAGE_PRE");
    lua_pushinteger(lua, NUKLEAR_STAGE_POST);
    lua_setglobal(lua, "NUKLEAR_STAGE_POST");

    lua_pushinteger(lua, NUKLEAR_MODE_SHARED);
    lua_setglobal(lua, "NUKLEAR_MODE_SHARED");
    lua_pushinteger(lua, NUKLEAR_MODE_EXCLUSIVE);
    lua_setglobal(lua, "NUKLEAR_MODE_EXCLUSIVE");

    int status = luaL_loadfile(lua, MakeDataPath("UI", "init.lua").c_str());
    if (status) {
        logger->Warning("Nuklear: couldn't load init template: %s", lua_tostring(lua, -1));
        lua_pop(lua, 1);
        return false;
    }

    int err = lua_pcall(lua, 0, 0, 0);
    if (lua_error_check(WINDOW_null, lua, err)) {
        logger->Warning("Nuklear: error loading init template: %s", lua_tostring(lua, -1));
        return false;
    }

    return true;
}
