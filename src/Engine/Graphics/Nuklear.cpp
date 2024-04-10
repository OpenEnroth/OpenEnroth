#include <memory>
#include <string>
#include <vector>

#include <lua.hpp>

#include <nuklear_config.h> // NOLINT: not a C system header.

#include "NuklearUtils.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/GameResourceManager.h"

#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/LodTextureCache.h"
#include "Engine/Party.h"

#include "GUI/GUIWindow.h"
#include "Library/Platform/Interface/PlatformEnums.h"

#include "Library/Serialization/Serialization.h"
#include "Library/Logger/Logger.h"
#include "Library/Logger/LogSink.h"

#include "Utility/DataPath.h"

lua_State *lua = nullptr;
Nuklear *nuklear = nullptr;

Nuklear::Nuklear() {
}

enum WIN_STATE {
    WINDOW_NOT_LOADED = 0,
    WINDOW_INITIALIZED,
    WINDOW_TEMPLATE_ERROR
};

struct hotkey {
    PlatformKey key;
    bool mod_control;
    bool mod_shift;
    bool mod_alt;
    int callback;
    WindowType winType;
};

struct nk_tex_font font_default;
std::vector<struct hotkey> hotkeys;

struct img {
    GraphicsImage *asset;
    struct nk_image nk;
};

struct context {
    const char *tmpl;
    int ui_draw;
    int ui_release;
    std::vector<struct img *> imgs;
    std::vector<struct nk_tex_font *> fonts;
    std::vector<void *> tmp;
    WindowType winType;
    WIN_STATE state;
    Nuklear::NUKLEAR_MODE mode;
};

struct context wins[WINDOW_DebugMenu + 1] = {};

enum lua_nk_color_type {
    LUA_COLOR_ANY,
    LUA_COLOR_RGBA,
    LUA_COLOR_RGBAF,
    LUA_COLOR_HSVA,
    LUA_COLOR_HSVAF
};

enum lua_nk_style_type {
    lua_nk_style_type_unknown,
    lua_nk_style_type_align_header,
    lua_nk_style_type_align_text,
    lua_nk_style_type_bool,
    lua_nk_style_type_color,
    lua_nk_style_type_cursor,
    lua_nk_style_type_edit,
    lua_nk_style_type_float,
    lua_nk_style_type_font,
    lua_nk_style_type_item,
    lua_nk_style_type_integer,
    lua_nk_style_type_symbol,
    lua_nk_style_type_vec2,
};

struct lua_nk_property {
    const char *property;
    void *ptr;
    enum lua_nk_style_type type;
};

struct lua_nk_style {
    const char *component;
    std::vector<struct lua_nk_property> props;
};

std::vector<struct lua_nk_style> lua_nk_styles;

#define PUSH_STYLE(element, component, property, type) element.push_back({ #property, &nuklear->ctx->style.component.property, type});
#define PUSH_BUTTON_STYLE(element, style, property, type) element.push_back({ #property, &style->property, type});

std::unique_ptr<Nuklear> Nuklear::Initialize() {
    std::unique_ptr<Nuklear> result = std::make_unique<Nuklear>();
    ::nuklear = result.get();

    result->ctx = new(struct nk_context);
    if (!result->ctx) {
        return nullptr;
    }

    if (!render->NuklearInitialize(&font_default)) {
        delete result->ctx;
        return nullptr;
    }

    for (int w = WINDOW_MainMenu; w != WINDOW_DebugMenu; w++) {
        wins[w].state = WINDOW_NOT_LOADED;
        wins[w].mode = NUKLEAR_MODE_SHARED;
        wins[w].ui_draw = -1;
        wins[w].ui_release = -1;
        wins[w].winType = (WindowType)w;
    }

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
    PUSH_STYLE(style.props, button, text_alignment, lua_nk_style_type_align_text);
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
    PUSH_STYLE(style.props, checkbox, text_alignment, lua_nk_style_type_align_text);
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
        PUSH_STYLE(style.props, combo, button.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, combo, button.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, combo, button.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, combo, button.normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, combo, button.text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, combo, button.text_alignment, lua_nk_style_type_align_text);
        PUSH_STYLE(style.props, combo, button.text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, combo, button.text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, combo, button.text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, combo, button.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, combo, button.image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, combo, button.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, combo, button.rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, combo, button.touch_padding, lua_nk_style_type_vec2);
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
    PUSH_STYLE(style.props, edit, active, lua_nk_style_type_item);
    PUSH_STYLE(style.props, edit, border_color, lua_nk_style_type_color);
    PUSH_STYLE(style.props, edit, hover, lua_nk_style_type_item);
    PUSH_STYLE(style.props, edit, normal, lua_nk_style_type_item);
    /* edit scrollbar */
    {
        PUSH_STYLE(style.props, edit, scrollbar.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, edit, scrollbar.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, edit, scrollbar.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, edit, scrollbar.normal, lua_nk_style_type_item);

        /* cursor */
        PUSH_STYLE(style.props, edit, scrollbar.cursor_active, lua_nk_style_type_cursor);
        PUSH_STYLE(style.props, edit, scrollbar.cursor_border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, edit, scrollbar.cursor_hover, lua_nk_style_type_cursor);
        PUSH_STYLE(style.props, edit, scrollbar.cursor_normal, lua_nk_style_type_cursor);

        /* properties */
        PUSH_STYLE(style.props, edit, scrollbar.border_cursor, lua_nk_style_type_float);
        PUSH_STYLE(style.props, edit, scrollbar.rounding_cursor, lua_nk_style_type_float);
        PUSH_STYLE(style.props, edit, scrollbar.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, edit, scrollbar.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, edit, scrollbar.rounding, lua_nk_style_type_float);

        /* optional buttons */
        PUSH_STYLE(style.props, edit.scrollbar, show_buttons, lua_nk_style_type_integer);
        {
            /* button background */
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.active, lua_nk_style_type_item);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.border_color, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.hover, lua_nk_style_type_item);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.normal, lua_nk_style_type_item);
            /* button text */
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.text_active, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.text_alignment, lua_nk_style_type_align_text);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.text_background, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.text_hover, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.text_normal, lua_nk_style_type_color);
            /* button properties */
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.border, lua_nk_style_type_float);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.image_padding, lua_nk_style_type_vec2);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.padding, lua_nk_style_type_vec2);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.rounding, lua_nk_style_type_float);
            PUSH_STYLE(style.props, edit, scrollbar.dec_button.touch_padding, lua_nk_style_type_vec2);
        }
        PUSH_STYLE(style.props, edit, scrollbar.dec_symbol, lua_nk_style_type_symbol);
        {
            /* button background */
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.active, lua_nk_style_type_item);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.border_color, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.hover, lua_nk_style_type_item);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.normal, lua_nk_style_type_item);
            /* button text */
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.text_active, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.text_alignment, lua_nk_style_type_align_text);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.text_background, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.text_hover, lua_nk_style_type_color);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.text_normal, lua_nk_style_type_color);
            /* button properties */
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.border, lua_nk_style_type_float);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.image_padding, lua_nk_style_type_vec2);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.padding, lua_nk_style_type_vec2);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.rounding, lua_nk_style_type_float);
            PUSH_STYLE(style.props, edit, scrollbar.inc_button.touch_padding, lua_nk_style_type_vec2);
        }
        PUSH_STYLE(style.props, edit, scrollbar.inc_symbol, lua_nk_style_type_symbol);
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
        PUSH_STYLE(style.props, property.dec_button, text_alignment, lua_nk_style_type_align_text);
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
        PUSH_STYLE(style.props, property.inc_button, text_alignment, lua_nk_style_type_align_text);
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
    PUSH_STYLE(style.props, selectable, text_alignment, lua_nk_style_type_align_text);
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
        PUSH_STYLE(style.props, slider, dec_button.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, slider, dec_button.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider, dec_button.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, slider, dec_button.normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, slider, dec_button.text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider, dec_button.text_alignment, lua_nk_style_type_align_text);
        PUSH_STYLE(style.props, slider, dec_button.text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider, dec_button.text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider, dec_button.text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, slider, dec_button.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, slider, dec_button.image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, slider, dec_button.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, slider, dec_button.rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, slider, dec_button.touch_padding, lua_nk_style_type_vec2);
    }
    /* inc_button */
    {
        /* button background */
        PUSH_STYLE(style.props, slider, inc_button.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, slider, inc_button.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider, inc_button.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, slider, inc_button.normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, slider, inc_button.text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider, inc_button.text_alignment, lua_nk_style_type_align_text);
        PUSH_STYLE(style.props, slider, inc_button.text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider, inc_button.text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, slider, inc_button.text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, slider, inc_button.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, slider, inc_button.image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, slider, inc_button.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, slider, inc_button.rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, slider, inc_button.touch_padding, lua_nk_style_type_vec2);
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
        PUSH_STYLE(style.props, tab, node_maximize_button.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab, node_maximize_button.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, node_maximize_button.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab, node_maximize_button.normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, tab, node_maximize_button.text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, node_maximize_button.text_alignment, lua_nk_style_type_align_text);
        PUSH_STYLE(style.props, tab, node_maximize_button.text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, node_maximize_button.text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, node_maximize_button.text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, tab, node_maximize_button.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab, node_maximize_button.image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab, node_maximize_button.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab, node_maximize_button.rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab, node_maximize_button.touch_padding, lua_nk_style_type_vec2);
    }
    /* node_minimize_button */
    {
        /* button background */
        PUSH_STYLE(style.props, tab, node_minimize_button.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab, node_minimize_button.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, node_minimize_button.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab, node_minimize_button.normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, tab, node_minimize_button.text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, node_minimize_button.text_alignment, lua_nk_style_type_align_text);
        PUSH_STYLE(style.props, tab, node_minimize_button.text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, node_minimize_button.text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, node_minimize_button.text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, tab, node_minimize_button.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab, node_minimize_button.image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab, node_minimize_button.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab, node_minimize_button.rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab, node_minimize_button.touch_padding, lua_nk_style_type_vec2);
    }
    /* tab_maximize_button */
    {
        /* button background */
        PUSH_STYLE(style.props, tab, tab_maximize_button.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab, tab_maximize_button.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, tab_maximize_button.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab, tab_maximize_button.normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, tab, tab_maximize_button.text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, tab_maximize_button.text_alignment, lua_nk_style_type_align_text);
        PUSH_STYLE(style.props, tab, tab_maximize_button.text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, tab_maximize_button.text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, tab_maximize_button.text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, tab, tab_maximize_button.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab, tab_maximize_button.image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab, tab_maximize_button.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab, tab_maximize_button.rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab, tab_maximize_button.touch_padding, lua_nk_style_type_vec2);
    }
    /* tab_minimize_button */
    {
        /* button background */
        PUSH_STYLE(style.props, tab, tab_minimize_button.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab, tab_minimize_button.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, tab_minimize_button.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, tab, tab_minimize_button.normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, tab, tab_minimize_button.text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, tab_minimize_button.text_alignment, lua_nk_style_type_align_text);
        PUSH_STYLE(style.props, tab, tab_minimize_button.text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, tab_minimize_button.text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, tab, tab_minimize_button.text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, tab, tab_minimize_button.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab, tab_minimize_button.image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab, tab_minimize_button.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, tab, tab_minimize_button.rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, tab, tab_minimize_button.touch_padding, lua_nk_style_type_vec2);
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
        PUSH_STYLE(style.props, window.header, close_button.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, window.header, close_button.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header, close_button.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, window.header, close_button.normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, window.header, close_button.text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header, close_button.text_alignment, lua_nk_style_type_align_text);
        PUSH_STYLE(style.props, window.header, close_button.text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header, close_button.text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header, close_button.text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, window.header, close_button.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, window.header, close_button.image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, window.header, close_button.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, window.header, close_button.rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, window.header, close_button.touch_padding, lua_nk_style_type_vec2);
    }
    /* minimize_button */
    {
        /* button background */
        PUSH_STYLE(style.props, window.header, minimize_button.active, lua_nk_style_type_item);
        PUSH_STYLE(style.props, window.header, minimize_button.border_color, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header, minimize_button.hover, lua_nk_style_type_item);
        PUSH_STYLE(style.props, window.header, minimize_button.normal, lua_nk_style_type_item);
        /* button text */
        PUSH_STYLE(style.props, window.header, minimize_button.text_active, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header, minimize_button.text_alignment, lua_nk_style_type_align_text);
        PUSH_STYLE(style.props, window.header, minimize_button.text_background, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header, minimize_button.text_hover, lua_nk_style_type_color);
        PUSH_STYLE(style.props, window.header, minimize_button.text_normal, lua_nk_style_type_color);
        /* button properties */
        PUSH_STYLE(style.props, window.header, minimize_button.border, lua_nk_style_type_float);
        PUSH_STYLE(style.props, window.header, minimize_button.image_padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, window.header, minimize_button.padding, lua_nk_style_type_vec2);
        PUSH_STYLE(style.props, window.header, minimize_button.rounding, lua_nk_style_type_float);
        PUSH_STYLE(style.props, window.header, minimize_button.touch_padding, lua_nk_style_type_vec2);
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
    PUSH_STYLE(style.props, window.header, align, lua_nk_style_type_align_header);
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

    wins[WINDOW_null].tmpl = "";
    wins[WINDOW_MainMenu].tmpl = "mainmenu";
    wins[WINDOW_MainMenu_Load].tmpl = "mainmenu_load";
    wins[WINDOW_GameUI].tmpl = "gameui";
    wins[WINDOW_DebugMenu].tmpl = "ui_debug_hud";

    return result;
}

bool Nuklear::Create(WindowType winType) {
    if (!nuklear || !nuklear->ctx)
        return false;

    if (!lua)
        LuaInit();

    return LuaLoadTemplate(winType);
}

bool lua_error_check(WindowType winType, lua_State *L, int err) {
    if (err != 0) {
        logger->error("Nuklear: [{}] LUA error: {}\n", wins[winType].tmpl, lua_tostring(L, -1));
        lua_pop(L, 1);
        return true;
    }

    return false;
}

int Nuklear::KeyEvent(PlatformKey key) {
    for (auto it = hotkeys.begin(); it < hotkeys.end(); it++) {
        struct hotkey hk = *it;
        if (hk.key == key) {
            if (hk.mod_control && !engine->keyboardInputHandler->IsKeyHeld(PlatformKey::KEY_CONTROL))
                continue;

            if (hk.mod_shift && !engine->keyboardInputHandler->IsKeyHeld(PlatformKey::KEY_SHIFT))
                continue;

            if (hk.mod_alt && !engine->keyboardInputHandler->IsKeyHeld(PlatformKey::KEY_ALT))
                continue;

            lua_rawgeti(lua, LUA_REGISTRYINDEX, hk.callback);
            lua_pushlightuserdata(lua, (void *)&wins[hk.winType]);
            int err = lua_pcall(lua, 1, 1, 0);
            if (lua_error_check(hk.winType, lua, err)) {
                return false;
            }
            return lua_toboolean(lua, -1) != 0;
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

void Nuklear::Release(WindowType winType, bool is_reload) {
    if (!nuklear || !nuklear->ctx)
        return;

    nk_clear(nuklear->ctx);

    if (wins[winType].tmpl && (wins[winType].state == WINDOW_INITIALIZED || wins[winType].state == WINDOW_TEMPLATE_ERROR)) {
        if (wins[winType].ui_release >= 0) {
            lua_rawgeti(lua, LUA_REGISTRYINDEX, wins[winType].ui_release);
            lua_pushlightuserdata(lua, (void *)&wins[winType]);
            int err = lua_pcall(lua, 1, 0, 0);
            if (lua_error_check(winType, lua, err)) {
                wins[winType].state = WINDOW_TEMPLATE_ERROR;
            }
        }

        int i = 0;
        for (auto it = wins[winType].imgs.begin(); it != wins[winType].imgs.end(); it++, i++) {
            if ((*it)->asset) {
                render->NuklearImageFree((*it)->asset);
                (*it)->asset->Release();
                logger->trace("Nuklear: [{}] asset {} unloaded", wins[winType].tmpl, i);
                delete *it;
            }
        }
        wins[winType].imgs.clear();
        wins[winType].imgs.swap(wins[winType].imgs);

        for (auto itf = wins[winType].fonts.begin(); itf != wins[winType].fonts.end(); itf++, i++) {
            delete *itf;
        }
        wins[winType].fonts.clear();
        wins[winType].fonts.swap(wins[winType].fonts);

        if (wins[winType].ui_draw >= 0) {
            luaL_unref(lua, LUA_REGISTRYINDEX, wins[winType].ui_draw);
            wins[winType].ui_draw = -1;
        }
        if (wins[winType].ui_release >= 0) {
            luaL_unref(lua, LUA_REGISTRYINDEX, wins[winType].ui_release);
            wins[winType].ui_release = -1;
        }

        if (!is_reload && (wins[winType].state == WINDOW_INITIALIZED || wins[winType].state == WINDOW_TEMPLATE_ERROR))
            wins[winType].state = WINDOW_NOT_LOADED;

        logger->trace("Nuklear: [{}] template unloaded", wins[winType].tmpl);
    } else {
        logger->warning("Nuklear: [{}] template is not loaded", wins[winType].tmpl);
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
        logger->info("Nuklear: hotkey '{}' is unset", GetDisplayName(hk.key));
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
    if (!nuklear)
        return;

    render->NuklearFontFree(&font_default);
    render->NuklearRelease();
    LuaRelease();
    for (auto it = lua_nk_styles.begin(); it < lua_nk_styles.end(); it++) {
        it->props.clear();
    }
    for (auto itk = hotkeys.begin(); itk < hotkeys.end(); itk++) {
        struct hotkey hk = *itk;
        luaL_unref(lua, LUA_REGISTRYINDEX, hk.callback);
        logger->info("Nuklear: hotkey '{}' is unset", GetDisplayName(hk.key));
    }
    hotkeys.clear();
    lua_nk_styles.clear();
    delete nuklear->ctx;
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
    if (!nuklear || !nuklear->ctx || !lua)
        return false;

    if (wins[winType].state == WINDOW_INITIALIZED) {
        if (wins[winType].ui_draw >= 0) {
            lua_rawgeti(lua, LUA_REGISTRYINDEX, wins[winType].ui_draw);
            lua_pushlightuserdata(lua, (void *)&wins[winType]);
            lua_pushinteger(lua, stage);
            int err = lua_pcall(lua, 2, 0, 0);
            lua_error_check(winType, lua, err);
        } else {
            wins[winType].state = WINDOW_TEMPLATE_ERROR;
        }

        if (stage == NUKLEAR_STAGE_POST) {
            nk_input_end(nuklear->ctx);
            //render->BeginScene2D();
            if (render->GetPresentDimensions() == render->GetRenderDimensions())
                render->NuklearRender(NK_ANTI_ALIASING_ON, NUKLEAR_MAX_VERTEX_MEMORY, NUKLEAR_MAX_ELEMENT_MEMORY);
            //render->EndScene();
            //render->Present();
            nk_input_begin(nuklear->ctx);

            for (auto it = wins[winType].tmp.begin(); it < wins[winType].tmp.end(); it++) {
                free(*it);
            }
            wins[winType].tmp.clear();
            wins[winType].tmp.swap(wins[winType].tmp);
        }
    }

    return true;
}

void Nuklear::LuaRelease() {
    if (!nuklear || !lua)
        return;

    lua_close(lua);
}

bool Nuklear::LuaLoadTemplate(WindowType winType) {
    std::string name;

    if (!wins[winType].tmpl) {
        logger->warning("Nuklear: [{}] unknown template", toString(winType).c_str());
        return false;
    }

    name = wins[winType].tmpl;
    int status = luaL_loadfile(lua, makeDataPath("ui", name + ".lua").c_str());
    if (status) {
        wins[winType].state = WINDOW_TEMPLATE_ERROR;
        logger->warning("Nuklear: [{}] couldn't load lua template: {}", wins[winType].tmpl, lua_tostring(lua, -1));
        lua_pop(lua, 1);
        return false;
    }

    lua_pushlightuserdata(lua, (void *)&wins[winType]);
    int err = lua_pcall(lua, 1, 0, 0);
    if (lua_error_check(winType, lua, err)) {
        wins[winType].state = WINDOW_TEMPLATE_ERROR;
        logger->warning("Nuklear: error loading template: {}", wins[winType].tmpl);
        return false;
    }

    lua_getfield(lua, LUA_GLOBALSINDEX, "ui_init");
    lua_pushlightuserdata(lua, (void *)&wins[winType]);
    err = lua_pcall(lua, 1, 1, 0);
    if (lua_error_check(winType, lua, err)) {
        wins[winType].state = WINDOW_TEMPLATE_ERROR;
        logger->warning("Nuklear: error executing template: {}", wins[winType].tmpl);
        return false;
    }

    if (lua_istable(lua, 1)) {
        lua_pushvalue(lua, 1);
        lua_pushnil(lua);
        while (lua_next(lua, -2)) {
            const char *key = luaL_checkstring(lua, -2);
            if (!strcmp(key, "mode")) {
                wins[winType].mode = (NUKLEAR_MODE)luaL_checkinteger(lua, -1);
            } else if (!strcmp(key, "draw") && lua_isfunction(lua, -1)) {
                lua_pushvalue(lua, -1);
                wins[winType].ui_draw = luaL_ref(lua, LUA_REGISTRYINDEX);
            } else if (!strcmp(key, "release") && lua_isfunction(lua, -1)) {
                lua_pushvalue(lua, -1);
                wins[winType].ui_release = luaL_ref(lua, LUA_REGISTRYINDEX);
            }

            lua_pop(lua, 1);
        }

        lua_pop(lua, 1);
    }
    lua_pop(lua, 1);

    if (wins[winType].ui_draw < 0 || wins[winType].ui_release < 0) {
        wins[winType].state = WINDOW_TEMPLATE_ERROR;
        logger->warning("Nuklear: error executing template: {}", wins[winType].tmpl);
        return false;
    }

    assert(lua_gettop(lua) == 0);
    wins[winType].state = WINDOW_INITIALIZED;

    logger->info("Nuklear: lua template '{}' loaded", name);

    return true;
}

static int lua_log_info(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 2));

    const char *str = lua_tostring(lua, 2);
    logger->info("Nuklear LUA: {}", str);

    return 0;
}

static int lua_log_warning(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 2));

    const char *str = lua_tostring(lua, 2);
    logger->warning("Nuklear LUA: {}", str);

    return 0;
}

static int lua_nk_parse_vec2(lua_State *L, int idx, struct nk_vec2 *vec) {
    if (lua_istable(L, idx)) {
        lua_pushvalue(L, idx);
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            lua_pushvalue(L, -2);
            int i = luaL_checkinteger(L, -1);
            switch (i) {
                case(1):
                    vec->x = (float)luaL_checknumber(L, -2);
                    break;
                case(2):
                    vec->y = (float)luaL_checknumber(L, -2);
                    break;
            }
            lua_pop(L, 2);
        }

        lua_pop(L, 1);
    } else {
        return luaL_argerror(L, idx, lua_pushfstring(L, "vec2 format is wrong"));
    }

    return 0;
}

static int lua_nk_is_hex(char c) {
    return (c >= '0' && c <= '9')
        || (c >= 'a' && c <= 'f')
        || (c >= 'A' && c <= 'F');
}

static int lua_nk_parse_ratio(lua_State *L, int idx, std::vector<float> *ratio) {
    if (lua_istable(L, idx)) {
        lua_pushvalue(L, idx);
        lua_pushnil(L);
        int count = 0;
        while (lua_next(L, -2)) {
            ratio->push_back((float)luaL_checknumber(L, -1));
            count++;

            lua_pop(L, 1);
        }

        lua_pop(L, 1);
    } else {
        return luaL_argerror(L, idx, lua_pushfstring(L, "ratio format is wrong"));
    }

    return 0;
}

static int lua_nk_parse_rect(lua_State *L, int idx, struct nk_rect *rect) {
    if (lua_istable(L, idx)) {
        lua_pushvalue(L, idx);
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            lua_pushvalue(L, -2);
            if (lua_isnumber(L, -1)) {
                int i = luaL_checkinteger(L, -1);
                switch (i) {
                case(1):
                    rect->x = (float)luaL_checknumber(L, -2);
                    break;
                case(2):
                    rect->y = (float)luaL_checknumber(L, -2);
                    break;
                case(3):
                    rect->w = (float)luaL_checknumber(L, -2);
                    break;
                case(4):
                    rect->h = (float)luaL_checknumber(L, -2);
                    break;
                }
            } else {
                const char *key = lua_tostring(L, -1);
                float* val = &rect->x;
                if (!strcmp(key, "x")) {
                    val = &rect->x;
                } else if (!strcmp(key, "y")) {
                    val = &rect->y;
                } else if (!strcmp(key, "w")) {
                    val = &rect->w;
                } else if (!strcmp(key, "h")) {
                    val = &rect->h;
                }

                if (!val) {
                    return luaL_argerror(L, idx, lua_pushfstring(L, "rect format is wrong"));
                }
                *val = (float)luaL_checknumber(L, -2);
            }
            lua_pop(L, 2);
        }

        lua_pop(L, 1);
    } else {
        return luaL_argerror(L, idx, lua_pushfstring(L, "rect format is wrong"));
    }

    return 0;
}


static int lua_nk_parse_align_header(lua_State *L, int idx, nk_flags *flag) {
    const char *strflag = luaL_checkstring(L, idx);

    if (!strcmp(strflag, "left"))
        *flag = NK_HEADER_LEFT;
    else if (!strcmp(strflag, "right"))
        *flag = NK_HEADER_RIGHT;
    else
        return luaL_argerror(L, idx, lua_pushfstring(L, "header align '%s' is unknown", strflag));

    return 0;
}

static int lua_nk_parse_align_text(lua_State *L, int idx, nk_flags *flag) {
    const char *strflag = luaL_checkstring(L, idx);

    if (!strcmp(strflag, "align_bottom"))
        *flag = NK_TEXT_ALIGN_BOTTOM;
    else if (!strcmp(strflag, "align_centered"))
        *flag = NK_TEXT_ALIGN_CENTERED;
    else if (!strcmp(strflag, "align_left"))
        *flag = NK_TEXT_ALIGN_LEFT;
    else if (!strcmp(strflag, "align_middle"))
        *flag = NK_TEXT_ALIGN_MIDDLE;
    else if (!strcmp(strflag, "align_right"))
        *flag = NK_TEXT_ALIGN_RIGHT;
    else if (!strcmp(strflag, "align_top"))
        *flag = NK_TEXT_ALIGN_TOP;
    else if (!strcmp(strflag, "centered"))
        *flag = NK_TEXT_CENTERED;
    else if (!strcmp(strflag, "left"))
        *flag = NK_TEXT_LEFT;
    else if (!strcmp(strflag, "right"))
        *flag = NK_TEXT_RIGHT;
    else
        return luaL_argerror(L, idx, lua_pushfstring(L, "text align '%s' is unknown", strflag));

    return 0;
}

static int lua_nk_parse_color(lua_State *L, int idx, nk_color *color, lua_nk_color_type type) {
    int r = -1, g = -1, b = -1, a = 255, h = -1, s = -1, v = -1;
    float f_r = -1, f_g = -1, f_b = -1, f_a = 255, f_h = -1, f_s = -1, f_v = -1;

    if (lua_istable(L, idx)) {
        lua_pushvalue(L, idx);
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            lua_pushvalue(L, -2);
            if (lua_isnumber(L, -1)) {
                int i = luaL_checkinteger(L, -1);
                switch (i) {
                    case(1):
                        r = luaL_checkinteger(L, -2);
                        break;
                    case(2):
                        g = luaL_checkinteger(L, -2);
                        break;
                    case(3):
                        b = luaL_checkinteger(L, -2);
                        break;
                    case(4):
                        a = luaL_checkinteger(L, -2);
                        break;
                }
            } else {
                const char *key = luaL_checkstring(L, -1);

                if (!strcmp(key, "r")) {
                    r = luaL_checkinteger(L, -2);
                } else if (!strcmp(key, "g")) {
                    g = luaL_checkinteger(L, -2);
                } else if (!strcmp(key, "b")) {
                    b = luaL_checkinteger(L, -2);
                } else if (!strcmp(key, "a")) {
                    a = luaL_checkinteger(L, -2);
                } else if (!strcmp(key, "h")) {
                    h = luaL_checkinteger(L, -2);
                } else if (!strcmp(key, "s")) {
                    s = luaL_checkinteger(L, -2);
                } else if (!strcmp(key, "v")) {
                    v = luaL_checkinteger(L, -2);
                } else if (!strcmp(key, "fr")) {
                    f_r = (float)luaL_checknumber(L, -2);
                } else if (!strcmp(key, "fg")) {
                    f_g = (float)luaL_checknumber(L, -2);
                } else if (!strcmp(key, "fb")) {
                    f_b = (float)luaL_checknumber(L, -2);
                } else if (!strcmp(key, "fa")) {
                    f_a = (float)luaL_checknumber(L, -2);
                } else if (!strcmp(key, "fh")) {
                    f_h = (float)luaL_checknumber(L, -2);
                } else if (!strcmp(key, "fs")) {
                    f_s = (float)luaL_checknumber(L, -2);
                } else if (!strcmp(key, "fv")) {
                    f_v = (float)luaL_checknumber(L, -2);
                }
            }

            lua_pop(L, 2);
        }

        lua_pop(L, 1);

    } else if (lua_isstring(L, idx)) {
        bool is_hex_color = true;
        const char *strcolor = luaL_checkstring(L, idx);
        size_t len = strlen(strcolor);
        if ((len == 7 || len == 9) && strcolor[0] == '#') {
            for (size_t i = 1; i < len; ++i) {
                if (!lua_nk_is_hex(strcolor[i])) {
                    is_hex_color = false;
                    break;
                }
            }

            if (is_hex_color) {
                sscanf(strcolor, "#%02x%02x%02x", &r, &g, &b);

                if (len == 9)
                    sscanf(strcolor + 7, "%02x", &a);
            }
        }
    }

    switch (type) {
        case(LUA_COLOR_RGBA):
            if (r >= 0 && g >= 0 && b >= 0 && a >= 0 && r <= 255 && g <= 255 && b <= 255 && a <= 255) {
                *color = nk_rgba(r, g, b, a);
                return 0;
            }
            break;
        case(LUA_COLOR_RGBAF):
            if (f_r >= 0 && f_g >= 0 && f_b >= 0 && f_a >= 0 && f_r <= 255 && f_g <= 255 && f_b <= 255 && f_a <= 255) {
                *color = nk_rgba_f(f_r, f_g, f_b, f_a);
                return 0;
            }
            break;
        case(LUA_COLOR_HSVA):
            if (h >= 0 && s >= 0 && v >= 0 && a >= 0 && h <= 255 && s <= 255 && v <= 255 && a <= 255) {
                *color = nk_hsva(h, s, v, a);
                return 0;
            }
        case(LUA_COLOR_HSVAF):
            if (f_h >= 0 && f_s >= 0 && f_v >= 0 && f_a >= 0 && f_h <= 255 && f_s <= 255 && f_v <= 255 && f_a <= 255) {
                *color = nk_hsva_f(f_h, f_s, f_v, f_a);
                return 0;
            }
        default:
            if (r >= 0 && g >= 0 && b >= 0 && a >= 0 && r <= 255 && g <= 255 && b <= 255 && a <= 255) {
                *color = nk_rgba(r, g, b, a);
                return 0;
            } else if (h >= 0 && s >= 0 && v >= 0 && a >= 0 && h <= 255 && s <= 255 && v <= 255 && a <= 255) {
                *color = nk_hsva(h, s, v, a);
                return 0;
            } else if (f_r >= 0 && f_g >= 0 && f_b >= 0 && f_a >= 0 && f_r <= 255 && f_g <= 255 && f_b <= 255 && f_a <= 255) {
                *color = nk_rgba_f(f_r, f_g, f_b, f_a);
                return 0;
            } else if (f_h >= 0 && f_s >= 0 && f_v >= 0 && f_a >= 0 && f_h <= 255 && f_s <= 255 && f_v <= 255 && f_a <= 255) {
                *color = nk_hsva_f(f_h, f_s, f_v, f_a);
                return 0;
            }
            break;
    }

    return luaL_argerror(L, idx, lua_pushfstring(L, "unrecognized color format"));
}

static int lua_nk_parse_color(lua_State *L, int idx, nk_color *color) {
    return lua_nk_parse_color(L, idx, color, LUA_COLOR_ANY);
}

static void lua_nk_push_color(lua_State *L, nk_color color) {
    float f_r, f_g, f_b, f_a, f_h, f_s, f_v;
    int h, s, v;

    nk_color_f(&f_r, &f_g, &f_b, &f_a, color);
    nk_color_hsv_i(&h, &s, &v, color);
    nk_color_hsv_f(&f_h, &f_s, &f_v, color);

    lua_newtable(L);
    lua_pushliteral(L, "r");
    lua_pushinteger(L, color.r);
    lua_rawset(L, -3);
    lua_pushliteral(L, "g");
    lua_pushinteger(L, color.g);
    lua_rawset(L, -3);
    lua_pushliteral(L, "b");
    lua_pushinteger(L, color.b);
    lua_rawset(L, -3);
    lua_pushliteral(L, "a");
    lua_pushinteger(L, color.a);
    lua_rawset(L, -3);

    lua_pushliteral(L, "fr");
    lua_pushnumber(L, f_r);
    lua_rawset(L, -3);
    lua_pushliteral(L, "fg");
    lua_pushnumber(L, f_g);
    lua_rawset(L, -3);
    lua_pushliteral(L, "fb");
    lua_pushnumber(L, f_b);
    lua_rawset(L, -3);
    lua_pushliteral(L, "fa");
    lua_pushnumber(L, f_a);
    lua_rawset(L, -3);

    lua_pushliteral(L, "h");
    lua_pushinteger(L, h);
    lua_rawset(L, -3);
    lua_pushliteral(L, "s");
    lua_pushinteger(L, s);
    lua_rawset(L, -3);
    lua_pushliteral(L, "v");
    lua_pushinteger(L, v);
    lua_rawset(L, -3);

    lua_pushliteral(L, "fh");
    lua_pushnumber(L, f_h);
    lua_rawset(L, -3);
    lua_pushliteral(L, "fs");
    lua_pushnumber(L, f_s);
    lua_rawset(L, -3);
    lua_pushliteral(L, "fv");
    lua_pushnumber(L, f_v);
    lua_rawset(L, -3);
}

static void lua_nk_push_colorf(lua_State *L, nk_colorf colorf) {
    nk_color color = nk_rgba_cf(colorf);

    lua_nk_push_color(L, color);
}

static int lua_nk_parse_image(struct context *w, lua_State *L, int idx, struct nk_image **image) {
    size_t slot = luaL_checkinteger(L, idx);
    if (slot >= 0 && slot < w->imgs.size() && w->imgs[slot]->asset) {
        *image = &w->imgs[slot]->nk;
        return 0;
    }

    return luaL_argerror(L, idx, lua_pushfstring(L, "asset is wrong"));
}

static int lua_nk_parse_image_asset(struct context *w, lua_State *L, int idx, GraphicsImage **asset) {
    size_t slot = luaL_checkinteger(L, idx);
    if (slot >= 0 && slot < w->imgs.size() && w->imgs[slot]->asset) {
        *asset = w->imgs[slot]->asset;
        return 0;
    }

    return luaL_argerror(L, idx, lua_pushfstring(L, "asset is wrong"));
}

static int lua_nk_parse_layout_format(lua_State *L, int idx, nk_layout_format *fmt) {
    const char *strfmt = luaL_checkstring(L, idx);
    if (!strcmp(strfmt, "dynamic")) {
        *fmt = NK_DYNAMIC;
    } else if (!strcmp(strfmt, "static")) {
        *fmt = NK_STATIC;
    } else {
        return luaL_argerror(L, idx, lua_pushfstring(L, "unrecognized layout format '%s'", strfmt));
    }

    return 0;
}

static int lua_nk_parse_modifiable(lua_State *L, int idx, nk_bool *modifiable) {
    const char *strmod = luaL_checkstring(L, idx);
    if (!strcmp(strmod, "modifiable")) {
        *modifiable = NK_MODIFIABLE;
    } else if (!strcmp(strmod, "fixed")) {
        *modifiable = NK_FIXED;
    } else {
        return luaL_argerror(L, idx, lua_pushfstring(L, "unrecognized modifiable '%s'", strmod));
    }

    return 0;
}

static int lua_nk_parse_popup_type(lua_State *L, int idx, nk_popup_type *type) {
    const char *strtype = luaL_checkstring(L, idx);
    if (!strcmp(strtype, "dynamic")) {
        *type = NK_POPUP_DYNAMIC;
    } else if (!strcmp(strtype, "static")) {
        *type = NK_POPUP_STATIC;
    } else {
        return luaL_argerror(L, idx, lua_pushfstring(L, "unrecognized popup type '%s'", strtype));
    }

    return 0;
}

static int lua_nk_parse_style(lua_State *L, int cidx, int pidx, lua_nk_style_type **type, void **ptr) {
    const char *component = luaL_checkstring(L, cidx);
    const char *property = luaL_checkstring(L, pidx);

    for (auto it = lua_nk_styles.begin(); it < lua_nk_styles.end(); it++) {
        struct lua_nk_style style = *it;
        // logger->Info("component: {}", style.component);
        if (!strcmp(component, style.component)) {
            for (auto itp = style.props.begin(); itp < style.props.end(); itp++) {
                struct lua_nk_property prop = *itp;
                // logger->Info("property: {}", prop.property);
                if (!strcmp(property, prop.property)) {
                    *type = &prop.type;
                    *ptr = prop.ptr;

                    return 0;
                }
            }

            return luaL_argerror(L, pidx, lua_pushfstring(L, "unknown property '%s'", property));
        }
    }

    return luaL_argerror(L, cidx, lua_pushfstring(L, "unknown component '%s", component));
}

static int lua_nk_parse_style_button(struct context *w, lua_State *L, int idx, nk_style_button *style) {
    nk_color color;
    float value;
    struct nk_image *img;
    nk_style_item item;
    std::vector<struct lua_nk_property> buttons_styles;

    PUSH_BUTTON_STYLE(buttons_styles, style, active, lua_nk_style_type_item);
    PUSH_BUTTON_STYLE(buttons_styles, style, border_color, lua_nk_style_type_color);
    PUSH_BUTTON_STYLE(buttons_styles, style, hover, lua_nk_style_type_item);
    PUSH_BUTTON_STYLE(buttons_styles, style, normal, lua_nk_style_type_item);
    PUSH_BUTTON_STYLE(buttons_styles, style, text_active, lua_nk_style_type_color);
    PUSH_BUTTON_STYLE(buttons_styles, style, text_alignment, lua_nk_style_type_align_text);
    PUSH_BUTTON_STYLE(buttons_styles, style, text_background, lua_nk_style_type_color);
    PUSH_BUTTON_STYLE(buttons_styles, style, text_hover, lua_nk_style_type_color);
    PUSH_BUTTON_STYLE(buttons_styles, style, text_normal, lua_nk_style_type_color);
    PUSH_BUTTON_STYLE(buttons_styles, style, border, lua_nk_style_type_float);
    PUSH_BUTTON_STYLE(buttons_styles, style, image_padding, lua_nk_style_type_vec2);
    PUSH_BUTTON_STYLE(buttons_styles, style, padding, lua_nk_style_type_vec2);
    PUSH_BUTTON_STYLE(buttons_styles, style, rounding, lua_nk_style_type_float);
    PUSH_BUTTON_STYLE(buttons_styles, style, touch_padding, lua_nk_style_type_vec2);

    // import current style for buttons from context
    for (auto it = lua_nk_styles.begin(); it < lua_nk_styles.end(); it++) {
        struct lua_nk_style s = *it;
        if (!strcmp(s.component, "button")) {
            for (auto itc = s.props.begin(); itc < s.props.end(); itc++) {
                struct lua_nk_property sprop = *itc;
                for (auto itb = buttons_styles.begin(); itb < buttons_styles.end(); itb++) {
                    struct lua_nk_property bprop = *itb;
                    if (!strcmp(sprop.property, bprop.property)) {
                        switch (sprop.type) {
                            case(lua_nk_style_type_align_text):
                                memcpy(bprop.ptr, sprop.ptr, sizeof(nk_flags));
                                break;
                            case(lua_nk_style_type_color):
                                memcpy(bprop.ptr, sprop.ptr, sizeof(nk_color));
                                break;
                            case(lua_nk_style_type_float):
                                memcpy(bprop.ptr, sprop.ptr, sizeof(float));
                                break;
                            case(lua_nk_style_type_item):
                                memcpy(bprop.ptr, sprop.ptr, sizeof(struct nk_style_item));
                                break;
                            case(lua_nk_style_type_vec2):
                                memcpy(bprop.ptr, sprop.ptr, sizeof(struct nk_vec2));
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            break;
        }
    }

    if (lua_istable(L, idx)) {
        lua_pushvalue(lua, idx);
        lua_pushnil(lua);
        while (lua_next(lua, -2)) {
            const char *key = luaL_checkstring(L, -2);

            for (auto itp = buttons_styles.begin(); itp < buttons_styles.end(); itp++) {
                struct lua_nk_property prop = *itp;
                if (!strcmp(key, prop.property)) {
                    switch (prop.type) {
                        case(lua_nk_style_type_align_text):
                            lua_check_ret(lua_nk_parse_align_text(L, -1, (nk_flags*)prop.ptr));
                            break;
                        case(lua_nk_style_type_color):
                            lua_check_ret(lua_nk_parse_color(L, -1, (nk_color*)prop.ptr));
                            break;
                        case(lua_nk_style_type_float):
                            value = (float)luaL_checknumber(L, -1);
                            memcpy(prop.ptr, &value, sizeof(float));
                            break;
                        case(lua_nk_style_type_item):
                            if (lua_isnumber(L, -1)) {
                                lua_check_ret(lua_nk_parse_image(w, L, -1, &img));
                                item = nk_style_item_image(*img);
                                memcpy(prop.ptr, &item, sizeof(nk_style_item));
                            } else if (lua_istable(L, -1) || lua_isstring(L, -1)) {
                                lua_check_ret(lua_nk_parse_color(L, -1, &color));
                                item = nk_style_item_color(color);
                                memcpy(prop.ptr, &item, sizeof(nk_style_item));
                            } else {
                                // TODO: nine slice
                                return luaL_argerror(L, -1, lua_pushfstring(L, "not implemented yet"));
                            }
                            break;
                        case(lua_nk_style_type_vec2):
                            lua_check_ret(lua_nk_parse_vec2(L, -1, (struct nk_vec2*)prop.ptr));
                            break;
                        default:
                            break;
                    }
                }
            }

            lua_pop(L, 1);
        }
    }

    return 0;
}

static int lua_nk_parse_edit_string_options(lua_State *L, int idx, nk_flags *flags) {
    *flags = NK_EDIT_FIELD;

    if (lua_istable(L, idx)) {
        lua_foreach(L, idx) {
            const char *flag = luaL_checkstring(L, -1);
            if (!strcmp(flag, "multiline"))
                *flags |= NK_EDIT_MULTILINE;
            else if (!strcmp(flag, "no_cursor"))
                *flags |= NK_EDIT_NO_CURSOR;
            else if (!strcmp(flag, "commit_on_enter"))
                *flags |= NK_EDIT_SIG_ENTER;
            else if (!strcmp(flag, "selectable"))
                *flags |= NK_EDIT_SELECTABLE;
            else if (!strcmp(flag, "goto_end_on_activate"))
                *flags |= NK_EDIT_GOTO_END_ON_ACTIVATE;
            else
                return luaL_argerror(L, idx, lua_pushfstring(L, "unrecognized edit string option '%s'", flag));
        }
    } else {
        return luaL_argerror(L, idx, lua_pushfstring(L, "wrong edit string option argument"));
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
            else
                return luaL_argerror(L, idx, lua_pushfstring(L, "unrecognized window flag '%s'", flag));
        }
    } else {
        return luaL_argerror(L, idx, lua_pushfstring(L, "wrong window flag argument"));
    }

    return 0;
}

static int lua_nk_parse_scroll(lua_State *L, int idx, nk_scroll *scroll) {
    if (lua_istable(L, idx)) {
        lua_pushvalue(L, idx);
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            lua_pushvalue(L, -2);
            int i = luaL_checkinteger(L, -1);
            switch (i) {
            case(1):
                scroll->x = (float)luaL_checknumber(L, -2);
                break;
            case(2):
                scroll->y = (float)luaL_checknumber(L, -2);
                break;
            }
            lua_pop(L, 2);
        }

        lua_pop(L, 1);
    } else {
        return luaL_argerror(L, idx, lua_pushfstring(L, "scroll format is wrong"));
    }

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
    else
        return luaL_argerror(L, idx, lua_pushfstring(L, "symbol '%s' is unknown", strsymbol));

    return 0;
}

static int lua_nk_parse_collapse_states(lua_State *L, int idx, nk_collapse_states *states) {
    const char *strstate = luaL_checkstring(L, idx);

    if (!strcmp(strstate, "minimized"))
        *states = NK_MINIMIZED;
    else if (!strcmp(strstate, "maximized"))
        *states = NK_MAXIMIZED;
    else
        return luaL_argerror(L, idx, lua_pushfstring(L, "state '%s' is unknown", strstate));

    return 0;
}

static int lua_nk_parse_tree_type(lua_State *L, int idx, nk_tree_type *type) {
    const char *strtype = luaL_checkstring(L, idx);

    if (!strcmp(strtype, "tab"))
        *type = NK_TREE_TAB;
    else if (!strcmp(strtype, "node"))
        *type = NK_TREE_NODE;
    else
        return luaL_argerror(L, idx, lua_pushfstring(L, "type '%s' is unknown", strtype));

    return 0;
}

static nk_scroll* lua_nk_check_scroll(lua_State *L, int idx) {
    void* userdata = luaL_checkudata(L, idx, "nk_scroll_mt");
    luaL_argcheck(L, userdata != nullptr, idx, "nk_scroll value expected");
    return (nk_scroll*)userdata;
}

static int lua_nk_begin(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 4 && lua_gettop(L) <= 5));

    const char *name, *title = NULL;
    nk_flags flags = 0;
    struct nk_rect rect;
    name = luaL_checkstring(L, 2);
    lua_check_ret(lua_nk_parse_rect(L, 3, &rect));
    lua_check_ret(lua_nk_parse_window_flags(L, 4, &flags));
    if (lua_gettop(L) == 5)
        title = luaL_checkstring(L, 5);

    bool ret = nk_begin_titled(nuklear->ctx, name, title ? title : name, rect, flags);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_button_color(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    struct nk_color color;
    lua_check_ret(lua_nk_parse_color(L, 2, &color));

    bool ret = nk_button_color(nuklear->ctx, color);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_button_image(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 2 && lua_gettop(L) <= 5 && lua_gettop(L) != 3));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    struct nk_style_button style = {};
    bool styled = false;
    nk_flags flag = 0;
    struct nk_image *img;
    const char *label = NULL;

    luaL_checkinteger(L, 2);
    lua_check_ret(lua_nk_parse_image(w, L, 2, &img));

    if (lua_gettop(L) >= 4 && lua_isstring(L, 3)) {
        label = luaL_checkstring(L, 3);
        lua_check_ret(lua_nk_parse_align_text(L, 4, &flag));
    }

    if (lua_gettop(L) == 5) {
        lua_check_ret(lua_nk_parse_style_button(w, L, 5, &style));
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
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 2 && lua_gettop(L) <= 3));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    struct nk_style_button style = {};
    const char *label = luaL_checkstring(L, 2);

    bool ret;
    if (lua_gettop(L) == 3) {
        lua_check_ret(lua_nk_parse_style_button(w, L, 3, &style));
        ret = nk_button_label_styled(nuklear->ctx, &style, label);
    } else {
        ret = nk_button_label(nuklear->ctx, label);
    }

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_button_set_behavior(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    enum nk_button_behavior behaviour;
    const char *strbehaviour = luaL_checkstring(L, 2);
    if (!strcmp(strbehaviour, "default"))
        behaviour = NK_BUTTON_DEFAULT;
    else if (!strcmp(strbehaviour, "repeater"))
        behaviour = NK_BUTTON_REPEATER;
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "behaviour '%s' is unknown", strbehaviour));

    nk_button_set_behavior(nuklear->ctx, behaviour);

    return 0;
}

static int lua_nk_button_symbol(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 2 && lua_gettop(L) <= 5 && lua_gettop(L) != 3));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    struct nk_style_button style = {};
    const char *label = NULL;
    nk_flags flag = 0;
    bool styled = false;
    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;

    lua_check_ret(lua_nk_parse_symbol(L, 2, &symbol))

    if (lua_gettop(L) >= 4 && lua_isstring(L, 3)) {
        label = luaL_checkstring(L, 3);
        lua_check_ret(lua_nk_parse_align_text(L, 4, &flag));
    }

    if (lua_gettop(L) == 5) {
        lua_check_ret(lua_nk_parse_style_button(w, L, 5, &style));
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

static int lua_nk_chart_begin(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    nk_chart_type type;
    const char *strtype = luaL_checkstring(L, 2);
    int num = luaL_checkinteger(L, 3);
    float min = (float)luaL_checknumber(L, 4);
    float max = (float)luaL_checknumber(L, 5);

    if (!strcmp(strtype, "lines"))
        type = NK_CHART_LINES;
    else if (!strcmp(strtype, "column"))
        type = NK_CHART_COLUMN;
    else if (!strcmp(strtype, "max"))
        type = NK_CHART_MAX;
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "type '%s' is unknown", strtype));

    bool ret = nk_chart_begin(nuklear->ctx, type, num, min, max);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_chart_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_chart_end(nuklear->ctx);

    return 0;
}

static int lua_nk_chart_push(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    float value = (float)luaL_checknumber(L, 2);

    nk_flags flags = nk_chart_push(nuklear->ctx, value);

    const char *str = "";
    if (flags & NK_CHART_CLICKED)
        str = "clicked";
    else if (flags & NK_CHART_HOVERING)
        str = "hovering";

    lua_pushstring(L, str);

    return 1;
}

static int lua_nk_checkbox(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *label = luaL_checkstring(L, 2);
    nk_bool value = lua_toboolean(L, 3);

    bool ret = nk_checkbox_label(nuklear->ctx, label, &value);
    lua_pushboolean(L, value);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_color_picker(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    nk_color color;
    nk_color_format format;
    const char *strformat = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_color(L, 2, &color));

    if (!strcmp(strformat, "rgb"))
        format = NK_RGB;
    else if (!strcmp(strformat, "rgba"))
        format = NK_RGBA;
    else
        return luaL_argerror(L, 3, lua_pushfstring(L, "wrong color format"));

    nk_colorf colorf = nk_color_cf(color);
    colorf = nk_color_picker(nuklear->ctx, colorf, format);

    lua_nk_push_colorf(L, colorf);

    return 1;
}

static int lua_nk_color_update(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    nk_color color;
    const char *strformat = luaL_checkstring(L, 3);

    if (!strcmp(strformat, "rgba")) {
        lua_check_ret(lua_nk_parse_color(L, 2, &color, LUA_COLOR_RGBA));
    } else if (!strcmp(strformat, "rgbaf")) {
        lua_check_ret(lua_nk_parse_color(L, 2, &color, LUA_COLOR_RGBAF));
    } else if (!strcmp(strformat, "hsva")) {
        lua_check_ret(lua_nk_parse_color(L, 2, &color, LUA_COLOR_HSVA));
    } else if (!strcmp(strformat, "hsvaf")) {
        lua_check_ret(lua_nk_parse_color(L, 2, &color, LUA_COLOR_HSVAF));
    } else {
        return luaL_argerror(L, 3, lua_pushfstring(L, "wrong color format"));
    }

    lua_nk_push_color(L, color);

    return 1;
}

static int lua_nk_combo(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    char **items;
    int count = 0;
    int selected = luaL_checkinteger(L, 3);
    int item_height = luaL_checkinteger(L, 4);
    struct nk_vec2 size;

    lua_check_ret(lua_nk_parse_vec2(L, 5, &size));

    if (lua_istable(L, 2)) {
        count = lua_objlen(L, 2);
        lua_pushvalue(L, 2);
        lua_pushnil(L);
        items = (char **)calloc(count, sizeof(char*));
        int i = 0;
        while (lua_next(L, -2)) {
            const char *val = luaL_checkstring(L, -1);

            items[i] = (char *)val;

            i++;
            lua_pop(L, 1);
        }

        w->tmp.push_back((void *)items);
    } else {
        return luaL_argerror(L, 2, lua_pushfstring(L, "wrong items"));
    }

    int ret = nk_combo(nuklear->ctx, (const char **)items, count, selected, item_height, size);

    lua_pushinteger(L, ret);

    return 1;
}

static int lua_nk_combo_begin_color(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    struct nk_color color;
    struct nk_vec2 size;

    lua_check_ret(lua_nk_parse_color(L, 2, &color));
    lua_check_ret(lua_nk_parse_vec2(L, 3, &size));

    bool ret = nk_combo_begin_color(nuklear->ctx, color, size);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_combo_begin_image(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 3 && lua_gettop(L) <= 4));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    struct nk_image *img;
    struct nk_vec2 size;
    const char *label = NULL;

    lua_check_ret(lua_nk_parse_image(w, L, 2, &img));
    lua_check_ret(lua_nk_parse_vec2(L, 3, &size));

    if (lua_gettop(L) == 4) {
        label = luaL_checkstring(L, 4);
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
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    struct nk_vec2 size;
    const char *label = luaL_checkstring(L, 2);
    lua_check_ret(lua_nk_parse_vec2(L, 3, &size));

    bool ret = nk_combo_begin_label(nuklear->ctx, label, size);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_combo_begin_symbol(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 3 && lua_gettop(L) <= 4));

    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;
    struct nk_vec2 size;
    const char *label = NULL;

    lua_check_ret(lua_nk_parse_symbol(L, 2, &symbol))
    lua_check_ret(lua_nk_parse_vec2(L, 3, &size));
    if (lua_gettop(L) == 4) {
        label = luaL_checkstring(L, 4);
    }

    bool ret;
    if (label)
        ret = nk_combo_begin_symbol_label(nuklear->ctx, label, symbol, size);
    else
        ret = nk_combo_begin_symbol(nuklear->ctx, symbol, size);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_combo_close(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_combo_close(nuklear->ctx);

    return 0;
}

static int lua_nk_combo_item_image(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    struct nk_image *img;
    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_image(w, L, 2, &img));
    lua_check_ret(lua_nk_parse_align_text(L, 4, &flag));

    bool ret = nk_combo_item_image_label(nuklear->ctx, *img, label, flag);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_image_dimensions(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    GraphicsImage *asset = NULL;
    luaL_checkinteger(L, 2);

    lua_check_ret(lua_nk_parse_image_asset(w, L, 2, &asset));

    lua_pushnumber(L, asset->height());
    lua_pushnumber(L, asset->width());

    return 2;
}

static int lua_nk_combo_item_label(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 2);
    lua_check_ret(lua_nk_parse_align_text(L, 3, &flag));

    bool ret = nk_combo_item_label(nuklear->ctx, label, flag);

    lua_pushboolean(L, ret);
    return 1;
}

static int lua_nk_combo_item_symbol(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;
    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_symbol(L, 2, &symbol))
    lua_check_ret(lua_nk_parse_align_text(L, 4, &flag));

    bool ret = nk_combo_item_symbol_label(nuklear->ctx, symbol, label, flag);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_combo_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_combo_end(nuklear->ctx);

    return 0;
}

static void lua_nk_push_edit_string_result_flag(lua_State *L, nk_flags flags) {
    static const auto editStringResults = std::array{
        NK_EDIT_ACTIVE,
        NK_EDIT_INACTIVE,
        NK_EDIT_ACTIVATED,
        NK_EDIT_DEACTIVATED,
        NK_EDIT_COMMITED,
    };
    lua_newtable(L);
    for (int i = 0; i < editStringResults.size(); ++i) {
        const auto& flag = editStringResults[i];
        if (flags & flag) {
            lua_pushinteger(L, flag);
            lua_pushboolean(L, true);
            lua_rawset(L, -3);
        }
    }
}

static int lua_nk_edit_string(lua_State *L) {
    /*
    * Very temp buffer solution, but easy to use on the lua side
    * edit_string is expecting a buffer which would require lua to provide
    * new ways to instantiate a buffer
    */
    static const int MAX_BUFFER_SIZE = 1024;
    static char buffer[MAX_BUFFER_SIZE] = { 0 };

    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    nk_flags flags = 0;
    lua_check_ret(lua_nk_parse_edit_string_options(L, 2, &flags));
    const char *text = lua_tostring(lua, 3);
    size_t len = NK_CLAMP(0, strlen(text), MAX_BUFFER_SIZE - 1);
    memcpy(buffer, text, len);
    buffer[len] = '\0';
    int bufferLength = strlen(buffer);

    nk_flags resultFlags = nk_edit_string_zero_terminated(nuklear->ctx, flags, buffer, MAX_BUFFER_SIZE - 1, nk_filter_default);

    lua_pushstring(lua, buffer);
    lua_nk_push_edit_string_result_flag(L, resultFlags);

    return 2;
}

static int lua_nk_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_end(nuklear->ctx);

    return 0;
}

static int lua_nk_group_begin(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *title = luaL_checkstring(L, 2);
    nk_flags flags;
    lua_check_ret(lua_nk_parse_window_flags(L, 3, &flags));

    bool ret = nk_group_begin(nuklear->ctx, title, flags);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_group_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_group_end(nuklear->ctx);

    return 0;
}

static int lua_nk_group_scrolled_begin(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    nk_scroll* scroll = lua_nk_check_scroll(L, 2);
    const char *title = luaL_checkstring(L, 3);
    nk_flags flags;
    lua_check_ret(lua_nk_parse_window_flags(L, 4, &flags));

    bool ret = nk_group_scrolled_begin(nuklear->ctx, scroll, title, flags);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_group_scrolled_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_group_scrolled_end(nuklear->ctx);

    return 0;
}

static int lua_nk_group_set_scroll(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    const char *id = luaL_checkstring(L, 2);
    size_t xoffset = luaL_checkinteger(L, 3);
    size_t yoffset = luaL_checkinteger(L, 4);
    nk_group_set_scroll(nuklear->ctx, id, xoffset, yoffset);

    return 0;
}

static int lua_nk_layout_reset_min_row_height(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));
    nk_layout_reset_min_row_height(nuklear->ctx);
    return 0;
}

static int lua_nk_load_font_from_file(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    const char *fontname = luaL_checkstring(L, 2);
    size_t fontsize = luaL_checkinteger(L, 3);
    std::string fontpath = makeDataPath("fonts", fontname);

    struct nk_tex_font *font = render->NuklearFontLoad(fontpath.c_str(), fontsize);
    if (font) {
        w->fonts.push_back(font);
    }

    lua_pushlightuserdata(L, font);

    return 1;
}

static int lua_nk_label(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 2 && lua_gettop(L) <= 3));

    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 2);
    if (lua_gettop(L) == 3)
        lua_check_ret(lua_nk_parse_align_text(L, 3, &flag));

    if (flag)
        nk_label(nuklear->ctx, label, flag);
    else
        nk_label_wrap(nuklear->ctx, label);

    return 0;
}

static int lua_nk_label_colored(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 3 && lua_gettop(L) <= 4));

    nk_flags flag = 0;
    nk_color color;
    const char *label = luaL_checkstring(L, 2);
    lua_check_ret(lua_nk_parse_color(L, 3, &color));
    if (lua_gettop(L) == 4)
        lua_check_ret(lua_nk_parse_align_text(L, 4, &flag));

    if (flag)
        nk_label_colored(nuklear->ctx, label, flag, color);
    else
        nk_label_colored_wrap(nuklear->ctx, label, color);

    return 0;
}

static int lua_nk_layout_row(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    struct context *w = (struct context*)lua_touserdata(L, 1);
    float height = (float)luaL_checknumber(L, 3);
    int cols = luaL_checkinteger(L, 4);
    std::vector<float> ratio;
    enum nk_layout_format fmt;
    lua_check_ret(lua_nk_parse_layout_format(L, 2, &fmt));
    lua_check_ret(lua_nk_parse_ratio(L, 5, &ratio));
    //TODO
    float *floats = (float *)calloc(ratio.size(), sizeof(float));

    int i = 0;
    for (auto it = ratio.begin(); it != ratio.end(); it++, i++) {
        floats[i] = *it;
    }
    ratio.clear();

    w->tmp.push_back(floats);

    nk_layout_row(nuklear->ctx, fmt, height, cols, floats);

    return 0;
}

static int lua_nk_layout_row_dynamic(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    float height = (float)luaL_checknumber(L, 2);
    int cols = luaL_checkinteger(L, 3);

    nk_layout_row_dynamic(nuklear->ctx, height, cols);

    return 0;
}

static int lua_nk_layout_row_static(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    float height = (float)luaL_checknumber(L, 2);
    float width = (float)luaL_checknumber(L, 3);
    int cols = luaL_checkinteger(L, 4);

    nk_layout_row_static(nuklear->ctx, height, width, cols);

    return 0;
}

static int lua_nk_layout_row_begin(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    enum nk_layout_format fmt;
    float height = (float)luaL_checknumber(L, 3);
    int cols = luaL_checkinteger(L, 4);
    lua_check_ret(lua_nk_parse_layout_format(L, 2, &fmt));

    nk_layout_row_begin(nuklear->ctx, fmt, height, cols);

    return 0;
}

static int lua_nk_layout_row_push(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    float ratio = (float)luaL_checknumber(L, 2);

    nk_layout_row_push(nuklear->ctx, ratio);

    return 0;
}

static int lua_nk_layout_row_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_layout_row_end(nuklear->ctx);

    return 0;
}

static int lua_nk_layout_space_begin(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    enum nk_layout_format fmt;
    float height = (float)luaL_checknumber(L, 3);
    int widget_count = luaL_checkinteger(L, 4);
    lua_check_ret(lua_nk_parse_layout_format(L, 2, &fmt));

    nk_layout_space_begin(nuklear->ctx, fmt, height, widget_count);

    return 0;
}

static int lua_nk_layout_space_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_layout_space_end(nuklear->ctx);

    return 0;
}

static int lua_nk_layout_space_push(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    struct nk_rect rect;
    lua_check_ret(lua_nk_parse_rect(L, 2, &rect));

    nk_layout_space_push(nuklear->ctx, rect);

    return 0;
}

static int lua_nk_style_default(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_style_default(nuklear->ctx);

    return 0;
}

static int lua_nk_menu_begin_image(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 4 && lua_gettop(L) <= 5));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    nk_flags flag = 0;
    struct nk_image *img;
    struct nk_vec2 size;
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_image(w, L, 2, &img));
    lua_check_ret(lua_nk_parse_vec2(L, 4, &size));

    if (lua_gettop(L) == 5) {
        lua_check_ret(lua_nk_parse_align_text(L, 5, &flag));
    }

    bool ret;
    if (flag > 0)
        ret = nk_menu_begin_image_label(nuklear->ctx, label, flag, *img, size);
    else
        ret = nk_menu_begin_image(nuklear->ctx, label, *img, size);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_menu_begin_label(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    nk_flags flag = 0;
    struct nk_vec2 size;
    const char *label = luaL_checkstring(L, 2);
    lua_check_ret(lua_nk_parse_align_text(L, 3, &flag));
    lua_check_ret(lua_nk_parse_vec2(L, 4, &size));

    bool ret = nk_menu_begin_label(nuklear->ctx, label, flag, size);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_menu_begin_symbol(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 4 && lua_gettop(L) <= 5));

    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;
    nk_flags flag = 0;
    struct nk_vec2 size;
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_symbol(L, 2, &symbol))
    lua_check_ret(lua_nk_parse_vec2(L, 4, &size));

    if (lua_gettop(L) == 5) {
        lua_check_ret(lua_nk_parse_align_text(L, 5, &flag));
    }

    bool ret;
    if (flag > 0)
        ret = nk_menu_begin_symbol_label(nuklear->ctx, label, flag, symbol, size);
    else
        ret = nk_menu_begin_symbol(nuklear->ctx, label, symbol, size);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_menu_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_menu_end(nuklear->ctx);

    return 0;
}

static int lua_nk_menu_item_image(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    nk_flags flag = 0;
    struct nk_image *img;
    struct nk_vec2 size;
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_image(w, L, 2, &img));
    lua_check_ret(lua_nk_parse_vec2(L, 4, &size));
    lua_check_ret(lua_nk_parse_align_text(L, 5, &flag));

    bool ret = nk_menu_item_image_label(nuklear->ctx, *img, label, flag);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_menu_item_label(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 2);
    lua_check_ret(lua_nk_parse_align_text(L, 3, &flag));

    bool ret = nk_menu_item_label(nuklear->ctx, label, flag);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_menu_item_symbol(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;
    nk_flags flag = 0;
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_symbol(L, 2, &symbol))
    lua_check_ret(lua_nk_parse_align_text(L, 4, &flag));

    bool ret = nk_menu_item_symbol_label(nuklear->ctx, symbol, label, flag);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_menubar_begin(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_menubar_begin(nuklear->ctx);

    return 0;
}

static int lua_nk_menubar_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_menubar_end(nuklear->ctx);

    return 0;
}

static int lua_nk_option_label(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *label = luaL_checkstring(L, 2);
    nk_bool active = lua_toboolean(L, 3);

    bool ret = nk_option_label(nuklear->ctx, label, active);

    lua_pushboolean(L, ret);

    return 1;
}


static int lua_nk_popup_begin(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    nk_popup_type type;
    nk_flags flags = 0;
    struct nk_rect rect;
    const char *title = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_popup_type(L, 2, &type));
    lua_check_ret(lua_nk_parse_window_flags(L, 4, &flags));
    lua_check_ret(lua_nk_parse_rect(L, 5, &rect));

    bool ret = nk_popup_begin(nuklear->ctx, type, title, flags, rect);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_popup_end(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_popup_end(nuklear->ctx);

    return 0;
}

static int lua_nk_progress(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    nk_size value = luaL_checkinteger(L, 2);
    nk_size max = luaL_checkinteger(L, 3);
    nk_bool modifiable;
    lua_check_ret(lua_nk_parse_modifiable(L, 4, &modifiable));

    bool ret = nk_progress(nuklear->ctx, &value, max, modifiable);

    lua_pushinteger(L, value);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_property_float(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 7));

    const char *title = luaL_checkstring(L, 2);
    float min = (float)luaL_checknumber(L, 3);
    float value = (float)luaL_checknumber(L, 4);
    float max = (float)luaL_checknumber(L, 5);
    float step = (float)luaL_checknumber(L, 6);
    float inc_per_pixel = (float)luaL_checknumber(L, 7);

    nk_property_float(nuklear->ctx, title, min, &value, max, step, inc_per_pixel);

    lua_pushnumber(L, value);

    return 1;
}

static int lua_nk_property_int(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 7));

    const char *title = luaL_checkstring(L, 2);
    int min = luaL_checkinteger(L, 3);
    int value = luaL_checkinteger(L, 4);
    int max = luaL_checkinteger(L, 5);
    int step = luaL_checkinteger(L, 6);
    float inc_per_pixel = (float)luaL_checknumber(L, 7);

    nk_property_int(nuklear->ctx, title, min, &value, max, step, inc_per_pixel);

    lua_pushinteger(L, value);

    return 1;
}

static int lua_nk_propertyd(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 7));

    const char *title = luaL_checkstring(L, 2);
    double min = luaL_checknumber(L, 3);
    double value = luaL_checknumber(L, 4);
    double max = luaL_checknumber(L, 5);
    double step = luaL_checknumber(L, 6);
    float inc_per_pixel = (float)luaL_checknumber(L, 7);

    double ret = nk_propertyd(nuklear->ctx, title, min, value, max, step, inc_per_pixel);

    lua_pushnumber(L, ret);

    return 1;
}

static int lua_nk_propertyf(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 7));

    const char *title = luaL_checkstring(L, 2);
    float min = (float)luaL_checknumber(L, 3);
    float value = (float)luaL_checknumber(L, 4);
    float max = (float)luaL_checknumber(L, 5);
    float step = (float)luaL_checknumber(L, 6);
    float inc_per_pixel = (float)luaL_checknumber(L, 7);

    float ret = nk_propertyf(nuklear->ctx, title, min, value, max, step, inc_per_pixel);

    lua_pushnumber(L, ret);

    return 1;
}

static int lua_nk_propertyi(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 7));

    const char *title = luaL_checkstring(L, 2);
    int min = luaL_checkinteger(L, 3);
    int value = luaL_checkinteger(L, 4);
    int max = luaL_checkinteger(L, 5);
    int step = luaL_checkinteger(L, 6);
    float inc_per_pixel = (float)luaL_checknumber(L, 7);

    int ret = nk_propertyi(nuklear->ctx, title, min, value, max, step, inc_per_pixel);

    lua_pushinteger(L, ret);

    return 1;
}
static int lua_nk_selectable_image(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 6));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    nk_flags flag = 0;
    struct nk_image *img;
    struct nk_vec2 size;
    nk_bool value = lua_toboolean(L, 6);
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_image(w, L, 2, &img));
    lua_check_ret(lua_nk_parse_vec2(L, 4, &size));
    lua_check_ret(lua_nk_parse_align_text(L, 5, &flag));

    bool ret = nk_selectable_image_label(nuklear->ctx, *img, label, flag, &value);

    lua_pushboolean(L, value);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_selectable_label(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    nk_flags flag = 0;
    nk_bool value = lua_toboolean(L, 4);
    const char *label = luaL_checkstring(L, 2);
    lua_check_ret(lua_nk_parse_align_text(L, 3, &flag));

    bool ret = nk_selectable_label(nuklear->ctx, label, flag, &value);

    lua_pushboolean(L, value);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_selectable_symbol(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;
    nk_flags flag = 0;
    nk_bool value = lua_toboolean(L, 5);
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_symbol(L, 2, &symbol))
    lua_check_ret(lua_nk_parse_align_text(L, 4, &flag));

    bool ret = nk_selectable_symbol_label(nuklear->ctx, symbol, label, flag, &value);

    lua_pushboolean(L, value);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_slide_float(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    float min = (float)luaL_checknumber(L, 2);
    float value = (float)luaL_checknumber(L, 3);
    float max = (float)luaL_checknumber(L, 4);
    float step = (float)luaL_checknumber(L, 5);

    float ret = nk_slide_float(nuklear->ctx, min, value, max, step);

    lua_pushnumber(L, ret);

    return 1;
}

static int lua_nk_slide_int(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    int min = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    int max = luaL_checkinteger(L, 4);
    int step = luaL_checkinteger(L, 5);

    int ret = nk_slide_int(nuklear->ctx, min, value, max, step);

    lua_pushinteger(L, ret);

    return 1;
}

static int lua_nk_slider_float(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    float min = (float)luaL_checknumber(L, 2);
    float value = (float)luaL_checknumber(L, 3);
    float max = (float)luaL_checknumber(L, 4);
    float step = (float)luaL_checknumber(L, 5);

    bool ret = nk_slider_float(nuklear->ctx, min, &value, max, step);

    lua_pushnumber(L, value);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_slider_int(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 5));

    int min = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    int max = luaL_checkinteger(L, 4);
    int step = luaL_checkinteger(L, 5);

    bool ret = nk_slider_int(nuklear->ctx, min, &value, max, step);

    lua_pushinteger(L, value);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_style_from_table(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    struct nk_color table[NK_COLOR_COUNT];
    struct nk_color color = { 0, 0, 0, 255 };

    if (lua_istable(L, 2)) {
        lua_foreach(L, 2) {
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
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    lua_nk_style_type *type;
    void *ptr = nullptr;
    nk_color *color;
    float *value;
    struct nk_vec2 *vec2;
    int i = 0;

    lua_check_ret(lua_nk_parse_style(L, 2, 3, &type, &ptr));

    bool ret = false;
    switch (*type) {
        case(lua_nk_style_type_color):
            color = (nk_color *)ptr;

            lua_nk_push_color(L, *color);
            return 1;
        case(lua_nk_style_type_float):
            value = (float *)ptr;

            lua_pushnumber(L, *value);
            return 1;
        case(lua_nk_style_type_item):
            // TODO: style item
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

        default:
            break;
    }

    return luaL_argerror(L, 3, lua_pushfstring(L, "not implemented yet"));
}

static int lua_nk_style_pop(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    lua_nk_style_type *type;
    void *ptr = nullptr;
    lua_check_ret(lua_nk_parse_style(L, 2, 3, &type, &ptr));

    bool ret;
    switch (*type) {
        case(lua_nk_style_type_align_header):
        case(lua_nk_style_type_align_text):
            ret = nk_style_pop_flags(nuklear->ctx);
            break;
        case(lua_nk_style_type_color):
            ret = nk_style_pop_color(nuklear->ctx);
            break;
        case(lua_nk_style_type_float):
            ret = nk_style_pop_float(nuklear->ctx);
            break;
        case(lua_nk_style_type_cursor):
        case(lua_nk_style_type_item):
        case(lua_nk_style_type_symbol):
            ret = nk_style_pop_style_item(nuklear->ctx);
            break;
        case(lua_nk_style_type_vec2):
            ret = nk_style_pop_vec2(nuklear->ctx);
            break;
        default:
            return luaL_argerror(L, 3, lua_pushfstring(L, "not implemented style property"));
    }

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_style_push(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    lua_nk_style_type *type;
    void *ptr = nullptr;
    nk_color color;
    float value;
    struct nk_image *img;
    struct nk_vec2 vec2;
    nk_flags flags;

    lua_check_ret(lua_nk_parse_style(L, 2, 3, &type, &ptr));

    bool ret;
    switch (*type) {
        case(lua_nk_style_type_align_header):
            lua_check_ret(lua_nk_parse_align_header(L, 4, &flags));
            ret = nk_style_push_flags(nuklear->ctx, (nk_flags *)ptr, flags);
            break;
        case(lua_nk_style_type_align_text):
            lua_check_ret(lua_nk_parse_align_text(L, 4, &flags));
            ret = nk_style_push_flags(nuklear->ctx, (nk_flags *)ptr, flags);
            break;
        case(lua_nk_style_type_color):
            lua_check_ret(lua_nk_parse_color(L, 4, &color));
            ret = nk_style_push_color(nuklear->ctx, (nk_color *)ptr, color);
            break;
        case(lua_nk_style_type_float):
            value = (float)luaL_checknumber(L, 4);
            ret = nk_style_push_float(nuklear->ctx, (float *)ptr, value);
            break;
        case(lua_nk_style_type_item):
            if (lua_isnumber(L, 4)) {
                lua_check_ret(lua_nk_parse_image(w, L, 4, &img));
                ret = nk_style_push_style_item(nuklear->ctx, (nk_style_item *)ptr, nk_style_item_image(*img));
            } else if (lua_istable(L, 4) || lua_isstring(L, 4)) {
                lua_check_ret(lua_nk_parse_color(L, 4, &color));
                ret = nk_style_push_style_item(nuklear->ctx, (nk_style_item *)ptr, nk_style_item_color(color));
            } else {
                // TODO: nine slice
                lua_dumpstack(L);
                return luaL_argerror(L, 4, lua_pushfstring(L, "not implemented yet"));
            }

            break;
        case(lua_nk_style_type_vec2):
            lua_check_ret(lua_nk_parse_vec2(L, 4, &vec2));
            ret = nk_style_push_vec2(nuklear->ctx, (struct nk_vec2 *)ptr, vec2);
            break;
        default:
            return luaL_argerror(L, 3, lua_pushfstring(L, "not implemented style property"));
    }

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_style_set(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    lua_nk_style_type *type;
    void *ptr = nullptr;
    nk_color color;
    float value;
    struct nk_image *img;
    nk_style_item item;

    lua_check_ret(lua_nk_parse_style(L, 2, 3, &type, &ptr));

    switch (*type) {
        case(lua_nk_style_type_align_header):
            lua_check_ret(lua_nk_parse_align_header(L, 4, (nk_flags *)ptr));
            break;
        case(lua_nk_style_type_align_text):
            lua_check_ret(lua_nk_parse_align_text(L, 4, (nk_flags *)ptr));
            break;
        case(lua_nk_style_type_color):
            lua_check_ret(lua_nk_parse_color(L, 4, (nk_color *)ptr));
            break;
        case(lua_nk_style_type_float):
            value = (float)luaL_checknumber(L, 4);
            memcpy(ptr, &value, sizeof(float));
            break;
        case(lua_nk_style_type_item):
            if (lua_isnumber(L, 4)) {
                lua_check_ret(lua_nk_parse_image(w, L, 4, &img));
                item = nk_style_item_image(*img);
                memcpy(ptr, &item, sizeof(nk_style_item));
            } else if (lua_istable(L, 4) || lua_isstring(L, 4)) {
                lua_check_ret(lua_nk_parse_color(L, 4, &color));
                item = nk_style_item_color(color);
                memcpy(ptr, &item, sizeof(nk_style_item));
            } else {
                // TODO: nine slice
                return luaL_argerror(L, 4, lua_pushfstring(L, "not implemented yet"));
            }

            break;
        case(lua_nk_style_type_vec2):
            lua_check_ret(lua_nk_parse_vec2(L, 4, (struct nk_vec2 *)ptr));
            break;
        default:
            return luaL_argerror(L, 3, lua_pushfstring(L, "not implemented style property"));
    }

    return 0;
}

static int lua_nk_style_set_font(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    struct nk_tex_font *font = (struct nk_tex_font *)lua_touserdata(L, 2);

    if (font)
        nk_style_set_font(nuklear->ctx, &font->font->handle);

    return 0;
}

static int lua_nk_style_set_font_default(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    struct context *w = (struct context *)lua_touserdata(L, 1);

    nk_style_set_font(nuklear->ctx, &font_default.font->handle);

    return 0;
}

static int lua_nk_spacer(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_spacer(nuklear->ctx);

    return 0;
}

static int lua_nk_tree_element_pop(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_tree_element_pop(nuklear->ctx);

    return 0;
}

static int lua_nk_tree_element_push(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    nk_tree_type type;
    nk_collapse_states states;
    nk_bool value;
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_tree_type(L, 2, &type));
    lua_check_ret(lua_nk_parse_collapse_states(L, 4, &states));

    // TODO: replace __LINE__ with wins context
    bool ret = nk_tree_element_push_hashed(nuklear->ctx, type, label, states, &value, label, strlen(label), __LINE__);

    lua_pushboolean(L, value);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_tree_pop(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    nk_tree_pop(nuklear->ctx);

    return 0;
}

static int lua_nk_tree_push(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    nk_tree_type type;
    nk_collapse_states states;
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_tree_type(L, 2, &type));
    lua_check_ret(lua_nk_parse_collapse_states(L, 4, &states));

    // TODO: replace __LINE__ with wins context
    bool ret = nk_tree_push_hashed(nuklear->ctx, type, label, states, label, strlen(label), __LINE__);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_tree_state_pop(lua_State *L) {
    assert(lua_gettop(L) == 0);

    nk_tree_state_pop(nuklear->ctx);

    return 0;
}

static int lua_nk_tree_state_push(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 4));

    nk_tree_type type;
    nk_collapse_states states;
    const char *label = luaL_checkstring(L, 3);
    lua_check_ret(lua_nk_parse_tree_type(L, 2, &type));
    lua_check_ret(lua_nk_parse_collapse_states(L, 4, &states));

    bool ret = nk_tree_state_push(nuklear->ctx, type, label, &states);

    lua_pushboolean(L, states == NK_MAXIMIZED ? true : false);
    lua_pushboolean(L, ret);

    return 2;
}

static int lua_nk_window_is_closed(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    const char *title = luaL_checkstring(L, 2);

    bool ret = nk_window_is_closed(nuklear->ctx, title);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_window_is_hidden(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    const char *title = luaL_checkstring(L, 2);

    bool ret = nk_window_is_hidden(nuklear->ctx, title);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_window_is_hovered(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));
    bool ret = nk_window_is_hovered(nuklear->ctx);

    lua_pushboolean(L, ret);

    return 1;
}

static int lua_nk_window_get_size(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    struct nk_vec2 size = nk_window_get_size(nuklear->ctx);

    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);

    return 2;
}

static int lua_nk_window_set_size(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *name = lua_tostring(L, 2);
    struct nk_vec2 size;
    lua_check_ret(lua_nk_parse_vec2(L, 3, &size));

    nk_window_set_size(nuklear->ctx, name, size);

    return 0;
}

static int lua_nk_window_set_position(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *name = lua_tostring(L, 2);
    struct nk_vec2 size;
    lua_check_ret(lua_nk_parse_vec2(L, 3, &size));

    nk_window_set_size(nuklear->ctx, name, size);

    return 0;
}

static int lua_nk_window_set_bounds(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 3));

    const char *name = lua_tostring(L, 2);
    struct nk_rect bounds;
    lua_check_ret(lua_nk_parse_rect(L, 3, &bounds));

    nk_window_set_bounds(nuklear->ctx, name, bounds);

    return 0;
}

static int lua_nk_window_get_position(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    struct nk_vec2 size = nk_window_get_position(nuklear->ctx);

    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);

    return 2;
}

static int lua_nk_load_image(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) >= 3 && lua_gettop(L) <= 5));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    const char *type = luaL_checkstring(L, 2);
    const char *name = luaL_checkstring(L, 3);
    const char *lod = nullptr;
    Color mask = colorTable.TealMask;
    if (lua_gettop(L) == 4)
        mask = Color::fromC16(luaL_checkinteger(L, 4));
    if (lua_gettop(L) == 5)
        lod = luaL_checkstring(L, 5);

    struct img *im = new(struct img);

    int slot = -1;
    bool ret = false;
    if (lod) {
        // TODO: load from custom lod when this functionality becomes available
        // LodReader lodReader(lod);
    } else {
        if (!strcmp(type, "pcx"))
            im->asset = assets->getImage_PCXFromIconsLOD(name);
        else if (!strcmp(type, "bmp"))
            im->asset = assets->getImage_ColorKey(name, mask);
    }

    if (!im->asset)
        goto finish;

    im->nk = render->NuklearImageLoad(im->asset);

    slot = w->imgs.size();
    w->imgs.push_back(im);

    logger->info("Nuklear: [{}] asset {}: '{}', type '{}' loaded!", w->tmpl, slot, name, type);

    lua_pushnumber(L, slot);

    return 1;

finish:
    delete im;
    logger->warning("Nuklear: [{}] asset '{}', type '{}' loading failed!", w->tmpl, name, type);

    lua_pushnil(L);

    return 1;
}


static int lua_window_dimensions(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    Sizei size = window->size();
    Sizei renderSize = render->GetRenderDimensions();

    lua_pushnumber(L, size.w);
    lua_pushnumber(L, size.h);
    lua_pushnumber(L, renderSize.w);
    lua_pushnumber(L, renderSize.h);

    return 4;
}

static int lua_set_hotkey(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 6));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    PlatformKey gameKey;
    const char *key = luaL_checkstring(L, 2);
    bool key_control = lua_toboolean(L, 3);
    bool key_shift = lua_toboolean(L, 4);
    bool key_alt = lua_toboolean(L, 5);
    if (!TryParseDisplayName(key, &gameKey)) {
        return luaL_argerror(L, 2, lua_pushfstring(L, "key '%s' is unknown", key));
    }

    if (lua_isfunction(L, 6)) {
        struct hotkey hk;
        hk.key = gameKey;
        hk.mod_control = key_control;
        hk.mod_shift = key_shift;
        hk.mod_alt = key_alt;
        hk.callback = luaL_ref(L, LUA_REGISTRYINDEX);
        hk.winType = w->winType;

        for (auto itk = hotkeys.begin(); itk < hotkeys.end(); itk++) {
            struct hotkey ithk = *itk;
            if (ithk.key == hk.key) {
                luaL_unref(L, LUA_REGISTRYINDEX, ithk.callback);
                ithk.callback = hk.callback;
                logger->info("Nuklear: hotkey '{}' is reset", key);

                return 0;
            }
        }

        hotkeys.push_back(hk);
        logger->info("Nuklear: hotkey '{}' is set", key);

        return 0;
    }

    return luaL_argerror(L, 3, lua_pushfstring(L, "callback is wrong"));
}

static int lua_unset_hotkey(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 2));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    PlatformKey gameKey;
    const char *key = luaL_checkstring(L, 2);
    if (!TryParseDisplayName(key, &gameKey)) {
        return luaL_argerror(L, 2, lua_pushfstring(L, "key '%s' is unknown", key));
    }

    for (auto itk = hotkeys.begin(); itk < hotkeys.end(); itk++) {
        struct hotkey hk = *itk;
        if (hk.winType == w->winType && gameKey == hk.key) {
            luaL_unref(lua, LUA_REGISTRYINDEX, hk.callback);
            logger->info("Nuklear: hotkey '{}' is unset", key);
            hotkeys.erase(itk);
            return 0;
        }
    }

    return 0;
}

static int lua_unset_hotkeys(lua_State *L) {
    lua_check_ret(lua_check_args(L, lua_gettop(L) == 1));

    struct context *w = (struct context *)lua_touserdata(L, 1);
    for (auto itk = hotkeys.begin(); itk < hotkeys.end();) {
        struct hotkey hk = *itk;
        if (hk.winType == w->winType) {
            luaL_unref(lua, LUA_REGISTRYINDEX, hk.callback);
            logger->info("Nuklear: hotkey '{}' is unset", GetDisplayName(hk.key));
            itk = hotkeys.erase(itk);
        } else {
            ++itk;
        }
    }

    return 0;
}

static int lua_nk_scroll_new(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) >= 1 && lua_gettop(L) <= 2));

    int x = 0, y = 0;
    if (lua_gettop(L) >= 1) {
        x = luaL_checkint(L, 1);
    }
    if (lua_gettop(L) >= 2) {
        y = luaL_checkint(L, 2);
    }
    nk_scroll* scroll = (nk_scroll*)lua_newuserdata(L, sizeof(nk_scroll));
    scroll->x = x;
    scroll->y = y;
    luaL_getmetatable(L, "nk_scroll_mt");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_nk_scroll_set(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) >= 3));

    nk_scroll* scroll = lua_nk_check_scroll(L, 1);
    scroll->x = luaL_checkint(L, 2);
    scroll->y = luaL_checkint(L, 3);
    return 0;
}

static int lua_dev_config_set(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) >= 2 && lua_gettop(L) <= 3));

    int valueIndex = 3;
    const char *configName{};
    AnyConfigEntry* configEntry{};
    if (lua_gettop(L) > 2) {
        const char *sectionName = luaL_checkstring(L, 1);
        configName = luaL_checkstring(L, 2);
        ConfigSection* section = engine->config->section(sectionName);
        if (section != nullptr) {
            configEntry = section->entry(configName);
        } else {
            return luaL_argerror(L, 2, lua_pushfstring(L, "invalid section name: '%s'", sectionName));
        }
    } else {
        configName = luaL_checkstring(L, 1);
        for (auto&& section : engine->config->sections()) {
            configEntry = section->entry(configName);
            if (configEntry != nullptr) {
                break;
            }
        }
        valueIndex = 2;
    }

    if (configEntry != nullptr) {
        configEntry->setString(luaL_checkstring(L, valueIndex));
    } else {
        return luaL_argerror(L, 1, lua_pushfstring(L, "invalid config entry name: '%s'", configName));
    }

    return 0;
}

static int lua_dev_config_get(lua_State *L) {
    lua_check_ret(lua_check_args_count(L, lua_gettop(L) >= 1 && lua_gettop(L) <= 2));

    const char *configName{};
    AnyConfigEntry* configEntry{};
    if (lua_gettop(L) > 1) {
        const char *sectionName = luaL_checkstring(L, 1);
        configName = luaL_checkstring(L, 2);
        ConfigSection* section = engine->config->section(sectionName);
        if (section != nullptr) {
            configEntry = section->entry(configName);
        } else {
            return luaL_argerror(L, 2, lua_pushfstring(L, "invalid section name: '%s'", sectionName));
        }
    } else {
        configName = luaL_checkstring(L, 1);
        for (auto&& section : engine->config->sections()) {
            configEntry = section->entry(configName);
            if (configEntry != nullptr) {
                break;
            }
        }
    }

    if (configEntry != nullptr) {
        lua_pushstring(L, configEntry->string().c_str());
    } else {
        return luaL_argerror(L, 1, lua_pushfstring(L, "invalid config entry name: '%s'", configName));
    }

    return 1;
}

static bool load_init_lua_file(const char *file) {
    int status = luaL_loadfile(lua, makeDataPath("ui", file).c_str());
    if (status) {
        logger->warning("Nuklear: couldn't load init template: {}", lua_tostring(lua, -1));
        lua_pop(lua, 1);
        return false;
    }

    int err = lua_pcall(lua, 0, 0, 0);
    if (lua_error_check(lua, err)) {
        return false;
    }

    lua_getfield(lua, LUA_GLOBALSINDEX, "ui_init");
    lua_pushlightuserdata(lua, (void *)&wins[WINDOW_null]);
    err = lua_pcall(lua, 1, 0, 0);
    if (lua_error_check(lua, err)) {
        return false;
    }

    return true;
}

static bool lua_init_lua_files(const std::vector<std::string> &files) {
    bool result = true;
    for (auto&& file : files) {
        result &= load_init_lua_file(file.c_str());
    }

    return result;
}

bool Nuklear::LuaInit() {
    lua = luaL_newstate();

    lua_gc(lua, LUA_GCSTOP, 0);
    luaL_openlibs(lua);
    lua_gc(lua, LUA_GCRESTART, -1);

    lua_getglobal(lua, "package");
    lua_pushfstring(lua, makeDataPath("ui", "?.lua").c_str());
    lua_setfield(lua, -2, "path");
    lua_pushstring(lua, "");
    lua_setfield(lua, -2, "cpath");
    lua_pop(lua, 1);

    static const luaL_Reg log[] = {
        { "info", lua_log_info },
        { "warning", lua_log_warning },
        { NULL, NULL }
    };

    luaL_newlib(lua, log);
    lua_setglobal(lua, "log");

    static const luaL_Reg ui[] = {
        { "nk_begin", lua_nk_begin },
        { "nk_button_color", lua_nk_button_color },
        { "nk_button_image", lua_nk_button_image },
        { "nk_button_label", lua_nk_button_label },
        { "nk_button_set_behavior", lua_nk_button_set_behavior },
        { "nk_button_symbol", lua_nk_button_symbol },
        { "nk_chart_begin", lua_nk_chart_begin },
        { "nk_chart_end", lua_nk_chart_end },
        { "nk_chart_push", lua_nk_chart_push },
        { "nk_checkbox", lua_nk_checkbox },
        { "nk_checkbox_label", lua_nk_checkbox },
        { "nk_color_picker", lua_nk_color_picker },
        { "nk_color_update", lua_nk_color_update },
        { "nk_combo", lua_nk_combo },
        { "nk_combo_begin_color", lua_nk_combo_begin_color },
        { "nk_combo_begin_image", lua_nk_combo_begin_image },
        { "nk_combo_begin_label", lua_nk_combo_begin_label },
        { "nk_combo_begin_symbol", lua_nk_combo_begin_symbol },
        { "nk_combo_close", lua_nk_combo_close },
        { "nk_combo_item_image", lua_nk_combo_item_image },
        { "nk_combo_item_label", lua_nk_combo_item_label },
        { "nk_combo_item_symbol", lua_nk_combo_item_symbol },
        { "nk_combo_end", lua_nk_combo_end },
        { "nk_edit_string", lua_nk_edit_string },
        { "nk_end", lua_nk_end },
        { "nk_group_begin", lua_nk_group_begin },
        { "nk_group_end", lua_nk_group_end },
        { "nk_group_scrolled_begin", lua_nk_group_scrolled_begin },
        { "nk_group_scrolled_end", lua_nk_group_scrolled_end },
        { "nk_group_set_scroll", lua_nk_group_set_scroll },
        { "nk_image_dimensions", lua_nk_image_dimensions },
        { "nk_label", lua_nk_label },
        { "nk_label_colored", lua_nk_label_colored },
        { "nk_load_font_from_file", lua_nk_load_font_from_file },
        { "nk_load_image", lua_nk_load_image },
        { "nk_layout_row", lua_nk_layout_row },
        { "nk_layout_row_begin", lua_nk_layout_row_begin },
        { "nk_layout_row_dynamic", lua_nk_layout_row_dynamic },
        { "nk_layout_row_push", lua_nk_layout_row_push },
        { "nk_layout_row_static", lua_nk_layout_row_static },
        { "nk_layout_row_end", lua_nk_layout_row_end },
        { "nk_layout_space_begin", lua_nk_layout_space_begin},
        { "nk_layout_space_end", lua_nk_layout_space_end },
        { "nk_layout_space_push", lua_nk_layout_space_push },
        { "nk_layout_reset_min_row_height", lua_nk_layout_reset_min_row_height },
        { "nk_menu_begin_image", lua_nk_menu_begin_image },
        { "nk_menu_begin_label", lua_nk_menu_begin_label },
        { "nk_menu_begin_symbol", lua_nk_menu_begin_symbol },
        { "nk_menu_end", lua_nk_menu_end },
        { "nk_menu_item_image", lua_nk_menu_item_image },
        { "nk_menu_item_label", lua_nk_menu_item_label },
        { "nk_menu_item_symbol", lua_nk_menu_item_symbol },
        { "nk_menubar_begin", lua_nk_menubar_begin },
        { "nk_menubar_end", lua_nk_menubar_end },
        { "nk_option_label", lua_nk_option_label },
        { "nk_popup_begin", lua_nk_popup_begin },
        { "nk_popup_end", lua_nk_popup_end },
        { "nk_progress", lua_nk_progress },
        { "nk_property_float", lua_nk_property_float },
        { "nk_property_int", lua_nk_property_int },
        { "nk_propertyd", lua_nk_propertyd },
        { "nk_propertyf", lua_nk_propertyf },
        { "nk_propertyi", lua_nk_propertyi },
        { "nk_selectable_image", lua_nk_selectable_image },
        { "nk_selectable_label", lua_nk_selectable_label },
        { "nk_selectable_symbol", lua_nk_selectable_symbol },
        { "nk_slide_float", lua_nk_slide_float },
        { "nk_slide_int", lua_nk_slide_int },
        { "nk_slider_float", lua_nk_slider_float },
        { "nk_slider_int", lua_nk_slider_int },
        { "nk_spacer", lua_nk_spacer },
        { "nk_style_default", lua_nk_style_default },
        { "nk_style_from_table", lua_nk_style_from_table },
        { "nk_style_get", lua_nk_style_get },
        { "nk_style_pop", lua_nk_style_pop },
        { "nk_style_push", lua_nk_style_push },
        { "nk_style_set", lua_nk_style_set },
        { "nk_style_set_font", lua_nk_style_set_font },
        { "nk_style_set_font_default", lua_nk_style_set_font_default },
        { "nk_tree_element_pop", lua_nk_tree_element_pop },
        { "nk_tree_element_push", lua_nk_tree_element_push },
        { "nk_tree_pop", lua_nk_tree_pop },
        { "nk_tree_push", lua_nk_tree_push },
        { "nk_tree_state_pop", lua_nk_tree_state_pop },
        { "nk_tree_state_push", lua_nk_tree_state_push },
        { "nk_window_is_closed", lua_nk_window_is_closed },
        { "nk_window_is_hidden", lua_nk_window_is_hidden },
        { "nk_window_is_hovered", lua_nk_window_is_hovered },
        { "nk_window_get_size", lua_nk_window_get_size },
        { "nk_window_set_size", lua_nk_window_set_size },
        { "nk_window_get_position", lua_nk_window_get_position },
        { "nk_window_set_position", lua_nk_window_set_position },
        { "nk_window_set_bounds", lua_nk_window_set_bounds },
        { NULL, NULL }
    };
    luaL_newlib(lua, ui);
    lua_setglobal(lua, "ui");

    static const luaL_Reg window[] = {
        { "dimensions", lua_window_dimensions },
        { NULL, NULL }
    };
    luaL_newlib(lua, window);
    lua_setglobal(lua, "window");

    static const luaL_Reg dev[] = {
        { "config_set", lua_dev_config_set },
        { "config_get", lua_dev_config_get },
        { NULL, NULL }
    };
    luaL_newlib(lua, dev);
    lua_setglobal(lua, "dev");

    static const luaL_Reg nk_scroll[] = {
        { "new", lua_nk_scroll_new },
        { "set", lua_nk_scroll_set },
        { NULL, NULL }
    };
    luaL_newlib(lua, nk_scroll);
    lua_setglobal(lua, "nk_scroll");

    luaL_newmetatable(lua, "nk_scroll_mt");
    lua_pushstring(lua, "__index");
    lua_pushvalue(lua, -2);
    lua_settable(lua, -3);
    luaL_openlib(lua, nullptr, nk_scroll, 0);
    lua_pop(lua, -1);

    static const luaL_Reg hotkeys[] = {
        { "set_hotkey", lua_set_hotkey },
        { "unset_hotkey", lua_unset_hotkey },
        { "unset_hotkeys", lua_unset_hotkeys },
        { NULL, NULL }
    };
    luaL_newlib(lua, hotkeys);
    lua_setglobal(lua, "hotkeys");

    for (auto&& callback : _initLuaLibCallbacks) {
        callback(lua);
    }

    lua_pushinteger(lua, NUKLEAR_STAGE_PRE);
    lua_setglobal(lua, "NUKLEAR_STAGE_PRE");
    lua_pushinteger(lua, NUKLEAR_STAGE_POST);
    lua_setglobal(lua, "NUKLEAR_STAGE_POST");

    lua_pushinteger(lua, NUKLEAR_MODE_SHARED);
    lua_setglobal(lua, "NUKLEAR_MODE_SHARED");
    lua_pushinteger(lua, NUKLEAR_MODE_EXCLUSIVE);
    lua_setglobal(lua, "NUKLEAR_MODE_EXCLUSIVE");

    lua_pushinteger(lua, NK_EDIT_ACTIVE);
    lua_setglobal(lua, "NK_EDIT_ACTIVE");
    lua_pushinteger(lua, NK_EDIT_INACTIVE);
    lua_setglobal(lua, "NK_EDIT_INACTIVE");
    lua_pushinteger(lua, NK_EDIT_ACTIVATED);
    lua_setglobal(lua, "NK_EDIT_ACTIVATED");
    lua_pushinteger(lua, NK_EDIT_DEACTIVATED);
    lua_setglobal(lua, "NK_EDIT_DEACTIVATED");
    lua_pushinteger(lua, NK_EDIT_COMMITED);
    lua_setglobal(lua, "NK_EDIT_COMMITED");

    return lua_init_lua_files(_initLuaFiles);
}

void Nuklear::addInitLuaFile(const char *lua_file) {
    _initLuaFiles.push_back(lua_file);
}

void Nuklear::addInitLuaLibs(std::function<void(lua_State *)> callback) {
    _initLuaLibCallbacks.push_back(callback);
}

bool Nuklear::isInitialized(WindowType winType) const {
    return wins[winType].state == WIN_STATE::WINDOW_INITIALIZED;
}

lua_State* Nuklear::getLuaState() {
    return lua;
}
