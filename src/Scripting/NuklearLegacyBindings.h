#pragma once

#include <Engine/Graphics/Renderer/NuklearOverlayRenderer.h>

#include <vector>

#include <sol/sol.hpp>
#include <nuklear_config.h> // NOLINT: not a C system header.

class GraphicsImage;
struct lua_State;

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

class NuklearLegacyBindings {
 public:
    static void initBindings(lua_State *lua);
    static void setContext(nk_context *context);

 private:
    struct img {
        GraphicsImage *asset;
        struct nk_image nk;
    };

    struct context {
        std::vector<struct img *> imgs;
        std::vector<struct nk_tex_font *> fonts;
    };

    static void initStyles();
    static void initNkScrollType(lua_State *lua);
    static int lua_nk_parse_vec2(lua_State *L, int idx, struct nk_vec2 *vec);
    static int lua_nk_is_hex(char c);
    static int lua_nk_parse_ratio(lua_State *L, int idx, std::vector<float> *ratio);
    static int lua_nk_parse_rect(lua_State *L, int idx, struct nk_rect *rect);
    static int lua_nk_parse_align_header(lua_State *L, int idx, nk_flags *flag);
    static int lua_nk_parse_align_text(lua_State *L, int idx, nk_flags *flag);
    static int lua_nk_parse_color(lua_State *L, int idx, nk_color *color, lua_nk_color_type type);
    static int lua_nk_parse_color(lua_State *L, int idx, nk_color *color);
    static int lua_nk_parse_image(struct context *w, lua_State *L, int idx, struct nk_image **image);
    static int lua_nk_parse_image_asset(struct context *w, lua_State *L, int idx, GraphicsImage **asset);
    static int lua_nk_parse_layout_format(lua_State *L, int idx, nk_layout_format *fmt);
    static int lua_nk_parse_modifiable(lua_State *L, int idx, nk_bool *modifiable);
    static int lua_nk_parse_popup_type(lua_State *L, int idx, nk_popup_type *type);
    static int lua_nk_parse_style(lua_State *L, int cidx, int pidx, lua_nk_style_type **type, void **ptr);
    static int lua_nk_parse_style_button(struct context *w, lua_State *L, int idx, nk_style_button *style);
    static int lua_nk_parse_edit_string_options(lua_State *L, int idx, nk_flags *flags);
    static int lua_nk_parse_window_flags(lua_State *L, int idx, nk_flags *flags);
    static int lua_nk_parse_scroll(lua_State *L, int idx, nk_scroll *scroll);
    static int lua_nk_parse_symbol(lua_State *L, int idx, nk_symbol_type *symbol);
    static int lua_nk_parse_collapse_states(lua_State *L, int idx, nk_collapse_states *states);
    static int lua_nk_parse_tree_type(lua_State *L, int idx, nk_tree_type *type);
    static int lua_nk_begin(lua_State *L);
    static int lua_nk_button_color(lua_State *L);
    static int lua_nk_button_image(lua_State *L);
    static int lua_nk_button_label(lua_State *L);
    static int lua_nk_button_set_behavior(lua_State *L);
    static int lua_nk_button_symbol(lua_State *L);
    static int lua_nk_chart_begin(lua_State *L);
    static int lua_nk_chart_end(lua_State *L);
    static int lua_nk_chart_push(lua_State *L);
    static int lua_nk_checkbox(lua_State *L);
    static int lua_nk_color_picker(lua_State *L);
    static int lua_nk_color_update(lua_State *L);
    static int lua_nk_combo(lua_State *L);
    static int lua_nk_combo_begin_color(lua_State *L);
    static int lua_nk_combo_begin_image(lua_State *L);
    static int lua_nk_combo_begin_label(lua_State *L);
    static int lua_nk_combo_begin_symbol(lua_State *L);
    static int lua_nk_combo_close(lua_State *L);
    static int lua_nk_combo_item_image(lua_State *L);
    static int lua_nk_image_dimensions(lua_State *L);
    static int lua_nk_combo_item_label(lua_State *L);
    static int lua_nk_combo_item_symbol(lua_State *L);
    static int lua_nk_combo_end(lua_State *L);
    static int lua_nk_edit_string(lua_State *L);
    static int lua_nk_end(lua_State *L);
    static int lua_nk_group_begin(lua_State *L);
    static int lua_nk_group_end(lua_State *L);
    static int lua_nk_group_scrolled_begin(lua_State *L);
    static int lua_nk_group_scrolled_end(lua_State *L);
    static int lua_nk_group_set_scroll(lua_State *L);
    static int lua_nk_layout_reset_min_row_height(lua_State *L);
    static int lua_nk_label(lua_State *L);
    static int lua_nk_label_colored(lua_State *L);
    static int lua_nk_layout_row_dynamic(lua_State *L);
    static int lua_nk_layout_row_static(lua_State *L);
    static int lua_nk_layout_row_begin(lua_State *L);
    static int lua_nk_layout_row_push(lua_State *L);
    static int lua_nk_layout_row_end(lua_State *L);
    static int lua_nk_layout_space_begin(lua_State *L);
    static int lua_nk_layout_space_end(lua_State *L);
    static int lua_nk_layout_space_push(lua_State *L);
    static int lua_nk_style_default(lua_State *L);
    static int lua_nk_menu_begin_image(lua_State *L);
    static int lua_nk_menu_begin_label(lua_State *L);
    static int lua_nk_menu_begin_symbol(lua_State *L);
    static int lua_nk_menu_end(lua_State *L);
    static int lua_nk_menu_item_image(lua_State *L);
    static int lua_nk_menu_item_label(lua_State *L);
    static int lua_nk_menu_item_symbol(lua_State *L);
    static int lua_nk_menubar_begin(lua_State *L);
    static int lua_nk_menubar_end(lua_State *L);
    static int lua_nk_option_label(lua_State *L);
    static int lua_nk_popup_begin(lua_State *L);
    static int lua_nk_popup_end(lua_State *L);
    static int lua_nk_progress(lua_State *L);
    static int lua_nk_property_float(lua_State *L);
    static int lua_nk_property_int(lua_State *L);
    static int lua_nk_propertyd(lua_State *L);
    static int lua_nk_propertyf(lua_State *L);
    static int lua_nk_propertyi(lua_State *L);
    static int lua_nk_selectable_image(lua_State *L);
    static int lua_nk_selectable_label(lua_State *L);
    static int lua_nk_selectable_symbol(lua_State *L);
    static int lua_nk_slide_float(lua_State *L);
    static int lua_nk_slide_int(lua_State *L);
    static int lua_nk_slider_float(lua_State *L);
    static int lua_nk_slider_int(lua_State *L);
    static int lua_nk_style_from_table(lua_State *L);
    static int lua_nk_style_get(lua_State *L);
    static int lua_nk_style_pop(lua_State *L);
    static int lua_nk_style_push(lua_State *L);
    static int lua_nk_style_set(lua_State *L);
    static int lua_nk_style_set_font(lua_State *L);
    static int lua_nk_style_set_font_default(lua_State *L);
    static int lua_nk_spacer(lua_State *L);
    static int lua_nk_tree_element_pop(lua_State *L);
    static int lua_nk_tree_element_push(lua_State *L);
    static int lua_nk_tree_pop(lua_State *L);
    static int lua_nk_tree_push(lua_State *L);
    static int lua_nk_tree_state_pop(lua_State *L);
    static int lua_nk_tree_state_push(lua_State *L);
    static int lua_nk_window_is_closed(lua_State *L);
    static int lua_nk_window_is_hidden(lua_State *L);
    static int lua_nk_window_is_hovered(lua_State *L);
    static int lua_nk_window_get_size(lua_State *L);
    static int lua_nk_window_set_size(lua_State *L);
    static int lua_nk_window_set_position(lua_State *L);
    static int lua_nk_window_set_bounds(lua_State *L);
    static int lua_nk_window_get_position(lua_State *L);
    static int lua_nk_load_image(lua_State *L);
    static int lua_nk_scroll_new(lua_State *L);
    static int lua_nk_scroll_set(lua_State *L);
    static void lua_nk_push_edit_string_result_flag(lua_State *L, nk_flags flags);
    static void lua_nk_push_color(lua_State *L, nk_color color);
    static void lua_nk_push_colorf(lua_State *L, nk_colorf colorf);
    static nk_scroll *lua_nk_check_scroll(lua_State *L, int idx);

    static struct nk_context *_context;
    static std::vector<struct lua_nk_style> _styles;
};
