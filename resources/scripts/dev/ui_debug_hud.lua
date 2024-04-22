-- Screen: Console
-- This is a script tight to the GUIWindow_DebugMenu flag
-- It takes care of showing the debug tools. Currently the only debug tool in use is the console

local Console = require "console"

local isWindowHovered = false
local baseColor = { 32, 32, 32, 255 }
local messageBkgColor = { 16, 16, 16, 255 }

local function getColorAlpha(col)
    local a = isWindowHovered and 200 or 64
    return { col[1], col[2], col[3], a }
end

local function historyPrev()
    if isWindowHovered then
        Console:navigateHistory(-1)
    end
end

local function historyNext()
    if isWindowHovered then
        Console:navigateHistory(1)
    end
end

---@param ctx NuklearContext
---@return table
---@diagnostic disable-next-line: name-style-check
function ui_init(ctx)
    Console.scroll:set(0, 0)
    if #Console.messages == 0 then
        Console:addMessage("Type \"help\" on the command line to get a list of all the commands", { 255, 255, 255, 128 })
    end
    hotkeys.setHotkey(ctx, "UP", false, false, false, historyPrev)
    hotkeys.setHotkey(ctx, "DOWN", false, false, false, historyNext)
    return {
        mode = NUKLEAR_MODE_SHARED,
        draw = ui_draw,
        release = ui_release
    }
end

---@param ctx NuklearContext
local function drawFooter(ctx)
    ui.nk_style_push(ctx, "button", "normal", getColorAlpha(baseColor))
    ui.nk_style_push(ctx, "edit", "normal", getColorAlpha(baseColor))
    ui.nk_style_push(ctx, "edit", "text_normal", Console:getTextColor())
    ui.nk_style_push(ctx, "edit", "text_active", Console:getTextColor())
    ui.nk_style_push(ctx, "edit", "text_hover", Console:getTextColor())

    ui.nk_layout_row_begin(ctx, "dynamic", 0, 3)
    ui.nk_layout_row_push(ctx, 0.05);
    if ui.nk_button_label(ctx, Console.isExpanded and "<" or ">") then
        Console.isExpanded = not Console.isExpanded
    end

    ui.nk_layout_row_push(ctx, 0.80);
    local text, state = ui.nk_edit_string(ctx, { "commit_on_enter" }, Console:getText())

    Console:updateText(text, state)

    ui.nk_layout_row_push(ctx, 0.15);
    if ui.nk_button_label(ctx, "Send") then
        if not Console.editTB.showPlaceholder then
            Console:send()
        end
    end

    ui.nk_layout_row_end(ctx)

    ui.nk_style_pop(ctx, "button", "normal")
    ui.nk_style_pop(ctx, "edit", "normal")
    ui.nk_style_pop(ctx, "edit", "text_normal")
    ui.nk_style_pop(ctx, "edit", "text_active")
    ui.nk_style_pop(ctx, "edit", "text_hover")
end

---
---@param ctx NuklearContext
local function drawConsole(ctx)
    local mainWinW, mainWinH = window.dimensions(ctx)
    Console:updateWindowSize(isWindowHovered, mainWinW, mainWinH)
    ui.nk_window_set_bounds(ctx, "Debug Console", Console.rect)

    ui.nk_style_push(ctx, "window", "fixed_background", getColorAlpha(messageBkgColor))
    if ui.nk_begin(ctx, "Debug Console", Console.rect, { "movable", "scalable" }) then
        isWindowHovered = ui.nk_window_is_hovered(ctx)

        if isWindowHovered then
            ui.nk_layout_row_dynamic(ctx, Console.rect.h - Console.footerHeight - 28, 1)
            ui.nk_style_push(ctx, "window", "min_row_height_padding", 1)
            if ui.nk_group_scrolled_begin(ctx, Console.scroll, "Messages", { "scrollbar" }) then
                ui.nk_layout_row_dynamic(ctx, 0, 1)
                for i = 1, #Console.messages do
                    local message = Console.messages[i]
                    if message.source ~= "log" or Console.logEnabled then
                        -- we must split the message every N characters because the wrap features on Nuklear does not work properly
                        local n = Console.separateEveryNCharacters
                        for j = 1, #message.text, n do
                            ---@cast n integer
                            local split = string.sub(message.text, j, j + n - 1)
                            ui.nk_label_colored(ctx, split, message.col)
                        end
                    end
                end
                ui.nk_group_scrolled_end(ctx)
            end
            ui.nk_style_pop(ctx, "window", "min_row_height_padding")

            ui.nk_layout_row_dynamic(ctx, 20, 1)
            Console.logEnabled = ui.nk_checkbox_label(ctx, "Show Log", Console.logEnabled)
        end

        drawFooter(ctx)
    end
    ui.nk_end(ctx)
    ui.nk_style_pop(ctx, "window", "fixed_background")

    if ui.nk_window_is_hidden(ctx, "Debug Console") then
        dev.setConfig("debug", "show_console", false)
    end
end

---
---@param ctx NuklearContext
---@param stage NuklearStageType
---@diagnostic disable-next-line: name-style-check
function ui_draw(ctx, stage)
    if stage == NUKLEAR_STAGE_PRE then
        local show = dev.getConfig("debug", "show_console")
        if show == "true" then
            drawConsole(ctx)
        end
    end
end

---
---@param ctx NuklearContext
---@diagnostic disable-next-line: name-style-check
function ui_release(ctx)
    hotkeys.unsetHotkeys(ctx)
end
