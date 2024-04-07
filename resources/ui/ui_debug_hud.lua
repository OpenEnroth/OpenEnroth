local console = require "console"
local commands = require "commands"

local is_window_hovered = false
local base_color = { 32, 32, 32, 255 }
local message_bkg_color = { 16, 16, 16, 255 }

local function get_color_alpha(col)
    local a = is_window_hovered and 200 or 64
    return { col[1], col[2], col[3], a }
end

local function history_prev()
    if is_window_hovered then
        console:navigate_history(-1)
    end
end

local function history_next()
    if is_window_hovered then
        console:navigate_history(1)
    end
end

function ui_init(ctx)
    console.scroll:set(0, 0)
    if table.getn(console.messages) == 0 then
        console:add_message("Type \"help\" on the command line to get a list of all the commands", {255,255,255,128})
    end
    hotkeys.set_hotkey(ctx, "UP", false, false, false, history_prev)
    hotkeys.set_hotkey(ctx, "DOWN", false, false, false, history_next)
    return {
        mode = NUKLEAR_MODE_SHARED,
        draw = ui_draw,
        release = ui_release
    }
end

function ui_draw(ctx, stage)
    if stage == NUKLEAR_STAGE_PRE then
        local show = dev.config_get("debug", "show_console")
        if show then
            draw_console(ctx)
        end
    end
end

function ui_release(ctx)
    hotkeys.unset_hotkeys(ctx)
end

function draw_console(ctx)
    local main_win_w, main_win_h = window.dimensions(ctx)
    console:update_window_size(is_window_hovered, main_win_w, main_win_h)
    ui.nk_window_set_bounds(ctx, "Debug Console", console.rect)
    
    ui.nk_style_push(ctx, "window", "fixed_background", get_color_alpha(message_bkg_color))
    if ui.nk_begin(ctx, "Debug Console", console.rect, {"movable", "scalable"}) then
        is_window_hovered = ui.nk_window_is_hovered(ctx)

        if is_window_hovered then
            ui.nk_layout_row_dynamic(ctx, console.rect.h - console.footer_height - 28, 1)
            ui.nk_style_push(ctx, "window", "min_row_height_padding", 1)
            if ui.nk_group_scrolled_begin(ctx, console.scroll, "Messages", {"scrollbar"}) then
                ui.nk_layout_row_dynamic(ctx, 0, 1)
                for i=1, table.getn(console.messages) do
                    local message = console.messages[i]
                    if message.source ~= "log" or console.log_enabled then
                        -- we must split the message every N characters because the wrap features on Nuklear does not work properly
                        local n = console.separate_every_n_characters
                        for i = 1, #message.text, n do
                            local split = string.sub(message.text, i, i + n - 1)
                            ui.nk_label_colored(ctx, split, message.col)
                        end
                    end
                end
                ui.nk_group_scrolled_end(ctx)
            end
            ui.nk_style_pop(ctx, "window", "min_row_height_padding")
            
            ui.nk_layout_row_dynamic(ctx, 20, 1)
            console.log_enabled = ui.nk_checkbox_label(ctx, "Show Log", console.log_enabled)
        end
        
        draw_footer(ctx)
    end
    ui.nk_end(ctx)
    ui.nk_style_pop(ctx, "window", "fixed_background")

    if ui.nk_window_is_hidden(ctx, "Debug Console") then
        dev.config_set("debug", "show_console", false)
    end
end

function draw_footer(ctx)
    ui.nk_style_push(ctx, "button", "normal", get_color_alpha(base_color))
    ui.nk_style_push(ctx, "edit", "normal", get_color_alpha(base_color))
    ui.nk_style_push(ctx, "edit", "text_normal", console:get_text_color())
    ui.nk_style_push(ctx, "edit", "text_active", console:get_text_color())
    ui.nk_style_push(ctx, "edit", "text_hover", console:get_text_color())

    ui.nk_layout_row_begin(ctx, "dynamic", 0, 3)
    ui.nk_layout_row_push(ctx, 0.05);
    if ui.nk_button_label(ctx, console.is_expanded and "<" or ">") then
        console.is_expanded = not console.is_expanded
    end

    ui.nk_layout_row_push(ctx, 0.80);
    local text, state = ui.nk_edit_string(ctx, {"commit_on_enter"}, console:get_text())
    
    console:update_text(text, state)

    ui.nk_layout_row_push(ctx, 0.15);
    if ui.nk_button_label(ctx, "Send") then
        if not console.edit_tb.show_placeholder then
            console:execute()
        end
    end

    ui.nk_layout_row_end(ctx)
    
    ui.nk_style_pop(ctx, "button", "normal")
    ui.nk_style_pop(ctx, "edit", "normal")
    ui.nk_style_pop(ctx, "edit", "text_normal")
    ui.nk_style_pop(ctx, "edit", "text_active")
    ui.nk_style_pop(ctx, "edit", "text_hover")
end