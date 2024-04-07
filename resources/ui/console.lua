--- Module that takes care of the state of the debug console

local commands = require "commands"
local logger = require "logger"

local SUCCESS_COLOR = { 64, 146, 222, 255 }
local DEFAULT_COLOR = { 160, 160, 160, 255 }
local DEBUG_COLOR = { 255, 255, 255, 128 }
local WARNING_COLOR = { 255, 255, 0, 255 }
local FAILURE_COLOR = { 255, 0, 0, 255 }
local CRITICAL_COLOR = { 255, 128, 0, 255 }
local PLACEHOLDER_COLOR = { 255, 255, 255, 128 }
local EDIT_TEXT_COLOR = { 255, 255, 255, 255 }

-- The console table contains the state of the console. From position to the number of messages
local console = {
    rect = {
        x = 8, 
        y = 8,
        w = 600,
        h = 480
    },
    edit_tb = {  -- state of the text box where the user type the command
        text = "",  -- current command being typed
        state = {}, -- state of the text box element ( active, deactivated and so on... )
        placeholder_text = "Write something here...", -- the placeholder text shown when the text box is empty
        show_placeholder = false, -- flag that tells if we need to show the placeholder during the current draw
        text_color = EDIT_TEXT_COLOR
    },
    messages = {}, -- each message being sent to the console is stored in this table
    history = {}, -- the history of commands being executed. Useful to navigate back to previously written commands
    history_index = 1, -- utility index which tells us the command we're navigating back to
    scroll = nk_scroll.new(0, 0), -- the scrollbar position
    is_expanded = true, -- flag telling us if the console has been expanded or not ( width is increased )
    padding = 5,
    footer_height = 50, -- the footer is the section containing the bottom part of the console ( command line + send button )
    log_enabled = false, -- flag that tells if the log messages should be displayed in the console
    max_messages_count = 400, -- to avoid storing all the messages we can set a limit
    max_history_count = 40, -- to avoid storing all the commands history we can set a limit
    character_width = 7.3, --hack for a lacking text wrapping support in nuklear
    separate_every_n_characters = 50, --hack for a lacking text wrapping support in nuklear
}

local function get_color_success(is_success)
    if is_success then
        return SUCCESS_COLOR
    else
        return FAILURE_COLOR
    end
end

console.add_history = function(console, text)
    table.insert(console.history, text)
    console.history_index = #console.history + 1
    
    local count = #console.history - console.max_history_count
    if count > 0 then
        for i = 1, count do
            table.remove(console.history, 1)
        end
    end
end

console.add_message = function(console, text, color, source)
    lines = {}
--  unfortunately Nuklear doesn't support newlines parsing inside label component so we'll manually split each message in chunks
    for s in text:gmatch("[^\r\n]+") do
        table.insert(console.messages, { text=s, col=color, source=source })
    end

    local count = #console.messages - console.max_messages_count
    if count > 0 then
        for i = 1, count do
            table.remove(console.messages, 1)
        end
    end
    console:scroll_to_end()
end

console.scroll_to_end = function(console)
    --  little hack to put the scrollbar at the bottom whenever we insert a new message
    console.scroll:set(0, #console.messages * 100)
end

console.execute = function(console)
    local text = console.edit_tb.text
    console:add_history(text)
    console:add_message(text, DEFAULT_COLOR)
    local message, is_success = commands.execute(text)
    console:add_message(message, get_color_success(is_success))
    console.edit_tb.text = ""
end

console.navigate_history = function(console, step)
    local index = console.history_index
    local size = #console.history

    index = index + step
    if index < 1 then
        index = 1
    elseif index > size + 1 then
        index = size + 1
    end

    console.history_index = index
    if index < size + 1 then
        console.edit_tb.text = console.history[index];
    else
        console.edit_tb.text = ""
    end
end

console.get_text = function(console)
    return console.edit_tb.text
end

console.get_text_color = function(console)
    return console.edit_tb.text_color
end

console.update_text = function(console, text, state)
    console.edit_tb.text = text
    console.edit_tb.state = state
    if state[NK_EDIT_COMMITED] and text ~= "" then
        console:execute()
    elseif state[NK_EDIT_INACTIVE] and text == "" then
        console.edit_tb.show_placeholder = true
        console.edit_tb.text = console.edit_tb.placeholder_text
    elseif state[NK_EDIT_ACTIVE] and console.edit_tb.show_placeholder then
        console.edit_tb.text = ""
        console.edit_tb.show_placeholder = false
    end

    console.edit_tb.text_color = console.edit_tb.show_placeholder and PLACEHOLDER_COLOR or EDIT_TEXT_COLOR
end

console.update_window_size = function(console, is_window_hovered, w, h)
    local console_width = console.is_expanded and 600 or 400
    if is_window_hovered then
        console.rect.x = console.padding
        console.rect.y = console.padding
        console.rect.w = console_width
        console.rect.h = h - console.padding * 2
    else
        console.rect.x = console.padding
        console.rect.y = h - console.footer_height - console.padding
        console.rect.w = console_width
        console.rect.h = console.footer_height
    end

    console.separate_every_n_characters = (console_width - 8) / console.character_width
end

local function get_color_by_log_level(level)
    if level == "trace" then return DEFAULT_COLOR
    elseif level == "debug" then return DEBUG_COLOR
    elseif level == "info" then return DEFAULT_COLOR
    elseif level == "warning" then return WARNING_COLOR
    elseif level == "error" then return FAILURE_COLOR
    elseif level == "critical" then return CRITICAL_COLOR
    end

    return DEFAULT_COLOR
end

local function on_log(level, message)
    console:add_message("["..level.."] "..message, get_color_by_log_level(level), "log")
end

logger.register(on_log)

return console
