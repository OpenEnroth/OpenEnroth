#include <algorithm>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"

#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Localization.h"

#include "Platform/OSWindow.h"
#include "Platform/Api.h"

#include "nuklear/nuklear_config.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024
#define MAX_RATIO_ELEMENTS 16

Nuklear *pNuklear;
lua_State *lua = nullptr;

enum WIN_STATE {
    WINDOW_NOT_LOADED = 0,
    WINDOW_INITIALIZED,
    WINDOW_TEMPLATE_ERROR
};

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

Nuklear::Nuklear() {
}

bool Nuklear::Initialize() {
    struct nk_context* ctx = (struct nk_context*)window->GetNuklearContext();
    if (!render->NuklearInitialize())
        return false;

    for (int w = WINDOW_MainMenu; w != WINDOW_DebugMenu; w++) {
        wins[w].state = WINDOW_NOT_LOADED;
        wins[w].mode = NUKLEAR_SHARED;
    }

    wins[WINDOW_MainMenu].tmpl = "mainmenu";
    wins[WINDOW_MainMenu_Load].tmpl = "mainmenu_load";

    return true;
}

bool Nuklear::Create(WindowType winType) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return false;

    if (!lua)
        LuaInit();

    return LuaLoadTemplate(winType);
}

void Nuklear::Release(WindowType winType) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return;

    nk_clear(ctx);

    if (wins[winType].tmpl && (wins[winType].state == WINDOW_INITIALIZED || wins[winType].state == WINDOW_TEMPLATE_ERROR)) {
        int i = 0;
        for (auto it = wins[winType].img.begin(); it != wins[winType].img.end(); it++,i++) {
            if ((*it)->asset->Release())
                render->log->Info("Nuklear: [%s] asset %d unloaded", wins[winType].tmpl, i);
            else
                render->log->Warning("Nuklear: [%s] asset %d unloading failed!", wins[winType].tmpl, i);
        }
        wins[winType].img.clear();
        wins[winType].img.swap(wins[winType].img);

        if (wins[winType].state == WINDOW_INITIALIZED)
            wins[winType].state = WINDOW_NOT_LOADED;

        render->log->Info("Nuklear: [%s] template unloaded", wins[winType].tmpl);
    } else {
        render->log->Warning("Nuklear: [%s] template is not loaded", wins[winType].tmpl);
    }
}

bool Nuklear::Reload(WindowType winType) {
    if (winType == WINDOW_null) {
        for (int w = WINDOW_MainMenu; w != WINDOW_DebugMenu; w++) {
            if (wins[w].state == WINDOW_INITIALIZED || wins[w].state == WINDOW_TEMPLATE_ERROR) {
                Release(WindowType(w));
                Create(WindowType(w));
            }
        }

        return true;
    } else {
        Release(winType);
        return Create(winType);
    }
}

void Nuklear::Destroy() {
    render->NuklearRelease();
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

int lua_handle_error(lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    luaL_traceback(L, L, msg, 2);
    lua_remove(L, -2);

    return 1;
}

bool lua_error_check(WindowType winType, lua_State *L, int err) {
    if (err != 0) {
        render->log->Warning("Nuklear: [%s] LUA error: %s\n", wins[winType].tmpl, lua_tostring(L, -1));
        lua_pop(L, 1);
        return true;
    }

    return false;
}

bool Nuklear::Draw(NUKLEAR_STAGE stage, WindowType winType, int id) {
    struct nk_context* ctx = (struct nk_context*)window->GetNuklearContext();
    if (!ctx)
        return false;

    if (!lua)
        return false;

    if (wins[winType].state == WINDOW_INITIALIZED) {
        lua_getfield(lua, LUA_GLOBALSINDEX, "draw");
        currentWin = winType;
        lua_pushinteger(lua, stage);
        int err = lua_pcall(lua, 1, 0, 0);
        if (lua_error_check(winType, lua, err)) {
            wins[winType].state = WINDOW_TEMPLATE_ERROR;
            Release(winType);
        }

        currentWin = WINDOW_null;
        if (stage == NUKLEAR_POST) {
            nk_input_end(ctx);
            render->NuklearRender(NK_ANTI_ALIASING_OFF, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
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
        render->log->Warning("Nuklear: [%s] unknown template", ToString(winType));
        return false;
    }

    name = wins[winType].tmpl;
    currentWin = winType;
    int status = luaL_loadfile(lua, MakeDataPath("UI", name + ".lua").c_str());
    if (status) {
        wins[winType].state = WINDOW_TEMPLATE_ERROR;
        render->log->Warning("Nuklear: [%s] couldn't load lua template: %s", wins[winType].tmpl, lua_tostring(lua, -1));
        lua_pop(lua, 1);
        return false;
    }

    int err = lua_pcall(lua, 0, 0, 0);
    if (lua_error_check(winType, lua, err)) {
        wins[winType].state = WINDOW_TEMPLATE_ERROR;
        render->log->Warning("Nuklear: [%s] error loading template: %s", wins[winType].tmpl, lua_tostring(lua, -1));
        Release(winType);
        return false;
    }
    currentWin = WINDOW_null;

    assert(lua_gettop(lua) == 0);
    wins[winType].mode = NUKLEAR_MODE(luaL_checkinteger(lua, 0));
    wins[winType].state = WINDOW_INITIALIZED;

    render->log->Info("Nuklear: lua template '%s' loaded", name.c_str());

    return true;
}

static int lua_log_info(lua_State *L) {
    assert(lua_gettop(L) >= 1);
    const char *str = lua_tostring(lua, 1);
    render->log->Info("Nuklear LUA: %s", str);

    return 0;
}

static int lua_log_warning(lua_State *L) {
    assert(lua_gettop(L) >= 1);
    const char *str = lua_tostring(lua, 1);
    render->log->Warning("Nuklear LUA: %s", str);

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

static int lua_nk_parse_style_button(lua_State *L, int idx, nk_style_button *style) {
    if (lua_istable(L, idx)) {
        lua_foreach(L, idx) {
            const char *key = luaL_checkstring(L, -2);
            if (!strcmp(key, "border")) {
                style->border = luaL_checknumber(L, -1);
            } else if (!strcmp(key, "border_color")) {
                lua_check_ret(lua_nk_parse_color(L, -1, &style->border_color));
                //style->border_color = nk_rgba(255, 0, 0, 0);
            } else if (!strcmp(key, "rounding")) {
                style->rounding = luaL_checknumber(L, -1);
            } else if (!strcmp(key, "padding")) {
                lua_check_ret(lua_nk_parse_vec2(L, -1, &style->padding));
            } else if (!strcmp(key, "image_padding")) {
                lua_check_ret(lua_nk_parse_vec2(L, -1, &style->image_padding))
            } else if (!strcmp(key, "touch_padding")) {
                lua_check_ret(lua_nk_parse_vec2(L, -1, &style->touch_padding))
            } else if (!strcmp(key, "hover")) {
                int slot = luaL_checknumber(L, -1);
                
                if (wins[currentWin].img[slot]->asset)
                    style->hover = nk_style_item_image(wins[currentWin].img[slot]->nk);
                else {
                    const char *msg = lua_pushfstring(L, "asset is wrong");
                    return luaL_argerror(L, -1, msg);
                }
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
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) >= 3 && lua_gettop(L) <= 4);

    const char *name, *title = NULL;
    nk_flags flags = 0;
    struct nk_rect rect;
    name = luaL_checkstring(L, 1);
    lua_check_ret(lua_nk_parse_rect(L, 2, &rect));
    lua_check_ret(lua_nk_parse_window_flags(L, 3, &flags));
    if (lua_gettop(L) == 4)
        title = luaL_checkstring(L, 4);

    int open = nk_begin_titled(ctx, name, title ? title : name, rect, flags);
    lua_pushboolean(L, open);
    return 1;
}

static int lua_nk_button_color(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) == 1);

    struct nk_color color;
    lua_check_ret(lua_nk_parse_color(L, 1, &color));

    int open = nk_button_color(ctx, color);
    lua_pushboolean(L, open);
    return 1;
}

static int lua_nk_button_image(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) >= 1 && lua_gettop(L) <= 4 && lua_gettop(L) != 2);

    struct nk_style_button style = {};
    bool styled = false;
    nk_flags flag = 0;
    int slot = luaL_checkinteger(L, 1);
    struct nk_image img;
    const char *label = NULL;

    if (wins[currentWin].img[slot]->asset)
        img = wins[currentWin].img[slot]->nk;
    else {
        const char *msg = lua_pushfstring(L, "asset is wrong");
        return luaL_argerror(L, -1, msg);
    }

    if (lua_gettop(L) >= 3 && lua_isstring(L, 2)) {
        label = lua_tostring(L, 2);
        lua_check_ret(lua_nk_parse_text_align(L, 3, &flag));
    }

    if (lua_gettop(L) == 4) {
        lua_check_ret(lua_nk_parse_style_button(L, 4, &style));
        styled = true;
    }

    int open;
    if (label != NULL && styled)
        open = nk_button_image_label_styled(ctx, &style, img, label, flag);
    else if (label != NULL)
        open = nk_button_image_label(ctx, img, label, flag);
    else if (styled)
        open = nk_button_image_styled(ctx, &style, img);
    else
        open = nk_button_image(ctx, img);

    lua_pushboolean(L, open);
    return 1;
}

static int lua_nk_button_label(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) >= 1 && lua_gettop(L) <= 2);

    struct nk_style_button style = {};
    const char *label = luaL_checkstring(L, 1);

    int open;
    if (lua_gettop(L) == 2) {
        lua_check_ret(lua_nk_parse_style_button(L, 2, &style));
        open = nk_button_label_styled(ctx, &style, label);
    } else {
        open = nk_button_label(ctx, label);
    }

    lua_pushboolean(L, open);
    return 1;
}

static int lua_nk_button_symbol(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) >= 1 && lua_gettop(L) <= 4 && lua_gettop(L) != 2);
    struct nk_style_button style = {};
    const char *label = NULL;
    nk_flags flag = 0;
    bool styled = false;
    nk_symbol_type symbol = nk_symbol_type::NK_SYMBOL_NONE;

    lua_check_ret(lua_nk_parse_symbol(L, 1, &symbol))

    if (lua_gettop(L) >= 3 && lua_isstring(L, 2)) {
        label = lua_tostring(L, 2);
        lua_check_ret(lua_nk_parse_text_align(L, 3, &flag));
    }

    if (lua_gettop(L) == 4) {
        lua_check_ret(lua_nk_parse_style_button(L, 4, &style));
        styled = true;
    }

    int open;
    if (label != NULL && styled)
        open = nk_button_symbol_label_styled(ctx, &style, symbol, label, flag);
    else if (label != NULL)
        open = nk_button_symbol_label(ctx, symbol, label, flag);
    else if (styled)
        open = nk_button_symbol_styled(ctx, &style, symbol);
    else
        open = nk_button_symbol(ctx, symbol);

    lua_pushboolean(L, open);
    return 1;
}

static int lua_nk_style_from_table(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

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

    nk_style_from_table(ctx, table);

    return 0;
}

static int lua_nk_end(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) == 0);

    nk_end(ctx);

    return 0;
}

static int lua_nk_load_font(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) == 2);
    const char *fontname = luaL_checkstring(L, 1);
    size_t fontsize = luaL_checknumber(L, 2);
    std::string fontpath = MakeDataPath("fonts", fontname);

    struct nk_font *font = render->NuklearLoadFont(fontpath.c_str(), fontsize);
    if (font)
        nk_style_set_font(ctx, &font->handle);

    lua_pushboolean(L, !!font);

    return 1;
}

static int lua_nk_layout_row(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

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

    enum nk_layout_format fmt;

    if (!strcmp(strfmt, "dynamic"))
        fmt = NK_DYNAMIC;
    else if (!strcmp(strfmt, "static"))
        fmt = NK_STATIC;
    else {
        free(floats);
        ratio.clear();
        const char *msg = lua_pushfstring(L, "unrecognized format flag '%s'", strfmt);
        return luaL_argerror(L, -1, msg);
    }

    nk_layout_row(ctx, fmt, height, cols, (const float*)floats);
    free(floats);
    ratio.clear();

    return 0;
}

static int lua_nk_layout_row_dynamic(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) == 2);
    float height = luaL_checknumber(L, 1);
    float cols = luaL_checknumber(L, 2);

    nk_layout_row_dynamic(ctx, height, cols);

    return 0;
}

static int lua_nk_layout_row_static(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) == 3);
    float height = luaL_checknumber(L, 1);
    float width = luaL_checknumber(L, 2);
    float cols = luaL_checknumber(L, 3);

    nk_layout_row_static(ctx, height, width, cols);

    return 0;
}

static int lua_nk_layout_row_begin(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) == 3);
    const char *strfmt = luaL_checkstring(L, 1);
    float height = luaL_checknumber(L, 2);
    float cols = luaL_checknumber(L, 3);

    enum nk_layout_format fmt;

    if (!strcmp(strfmt, "dynamic"))
        fmt = NK_DYNAMIC;
    else
        fmt = NK_STATIC;

    nk_layout_row_begin(ctx, fmt, height, cols);

    return 0;
}

static int lua_nk_layout_row_push(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) == 2);
    const char *strfmt = luaL_checkstring(L, 1);
    float ratio = luaL_checknumber(L, 2);

    nk_layout_row_push(ctx, ratio);

    return 0;
}

static int lua_nk_layout_row_end(lua_State *L) {
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        return 0;

    assert(lua_gettop(L) == 0);

    nk_layout_row_end(ctx);

    return 0;
}

static int lua_nk_load_image(lua_State *L) {
    bool ret = false;
    struct nk_context *ctx = (struct nk_context *)window->GetNuklearContext();
    if (!ctx)
        goto finish;

    assert(lua_gettop(L) >= 2);
    const char *type = luaL_checkstring(L, 1);
    const char *name = luaL_checkstring(L, 2);
    int mask = 0x7FF;
    if (lua_gettop(L) == 3)
        mask = luaL_checkinteger(L, 3);

    struct img *im = new(struct img);

    if (!strcmp(type, "pcx"))
        im->asset = assets->GetImage_PCXFromIconsLOD(name);
    else if (!strcmp(type, "bmp"))
        im->asset = assets->GetImage_ColorKey(name, mask);
    else
        goto finish;

    if (!im->asset)
        goto finish;

    im->nk = render->NuklearImageLoad(im->asset, 0, 0);

    int slot = wins[currentWin].img.size();
    wins[currentWin].img.push_back(im);

    ret = true;

finish:
    if (ret)
        render->log->Info("Nuklear: [%s] asset %d: '%s', type '%s' loaded!", wins[currentWin].tmpl, slot, name, type);
    else
        render->log->Warning("Nuklear: [%s] asset '%s', type '%s' loading failed!", wins[currentWin].tmpl, name, type);

    lua_pushnumber(L, slot);
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

bool Nuklear::LuaInit() {
    lua = luaL_newstate();

    lua_gc(lua, LUA_GCSTOP, 0);
    luaL_openlibs(lua);
    lua_gc(lua, LUA_GCRESTART, -1);

    static const luaL_Reg log[] =
    {
        { "info", lua_log_info },
        { "warning", lua_log_warning },
        { NULL, NULL }
    };

    luaL_newlib(lua, log);
    lua_setglobal(lua, "log");

    static const luaL_Reg nk[] =
    {
        { "nk_begin", lua_nk_begin },
        { "nk_button_color", lua_nk_button_color },
        { "nk_button_image", lua_nk_button_image },
        { "nk_button_label", lua_nk_button_label },
        { "nk_button_symbol", lua_nk_button_symbol },
        { "nk_end", lua_nk_end },
        { "nk_load_font", lua_nk_load_font },
        { "nk_load_image", lua_nk_load_image },
        { "nk_layout_row", lua_nk_layout_row },
        { "nk_layout_row_begin", lua_nk_layout_row_begin },
        { "nk_layout_row_dynamic", lua_nk_layout_row_dynamic },
        { "nk_layout_row_push", lua_nk_layout_row_push },
        { "nk_layout_row_static", lua_nk_layout_row_static },
        { "nk_layout_row_end", lua_nk_layout_row_end },
        { "nk_style_from_table", lua_nk_style_from_table },
        { NULL, NULL }
    };
    luaL_newlib(lua, nk);
    lua_setglobal(lua, "ui");

    static const luaL_Reg dim[] =
    {
        { "dimensions", lua_window_dimensions },
        { NULL, NULL }
    };
    luaL_newlib(lua, dim);
    lua_setglobal(lua, "window");

    return true;
}
