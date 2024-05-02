--- Module that takes care of the state of the debug console

local CommandManager = require "dev.commands.command_manager"
---@type Nuklear
local nk = require "bindings.overlay".nk
local logListener = require "core.logger".listener

local successColor = { 64, 146, 222, 255 }
local defaultColor = { 160, 160, 160, 255 }
local debugColor = { 255, 255, 255, 128 }
local warningColor = { 255, 255, 0, 255 }
local failureColor = { 255, 0, 0, 255 }
local criticalColor = { 255, 128, 0, 255 }
local placeHolderColor = { 255, 255, 255, 128 }
local editTextColor = { 255, 255, 255, 255 }

---@class ConsoleMessage
---@field text string
---@field col table Color to use for the text
---@field source string Tell from where the message has been sent

---@class Console The console table contains the state of the console. From position to the number of messages
---@field messages table<integer, ConsoleMessage> List of all the messages displayed in the console
---@field maxMessagesCount integer Maximum number of messages the console can show. After that the oldest messages are removed
---@field history table List of all the messages sent by the user
---@field historyIndex integer current message being shown ( used to go back/forward when the user want to execute send messages )
---@field separateEveryNCharacters number --hack for a lacking text wrapping support in nuklear
---@field isExpanded boolean
---@field logEnabled boolean
local Console = {
    rect = {
        x = 8,
        y = 8,
        w = 600,
        h = 480
    },
    editTB = {                                       -- state of the text box where the user type the command
        text = "",                                   -- current command being typed
        state = {},                                  -- state of the text box element ( active, deactivated and so on... )
        placeholderText = "Write something here...", -- the placeholder text shown when the text box is empty
        showPlaceholder = false,                     -- flag that tells if we need to show the placeholder during the current draw
        textColor = editTextColor
    },
    messages = {},                 -- each message being sent to the console is stored in this table
    history = {},                  -- the history of commands being executed. Useful to navigate back to previously written commands
    historyIndex = 1,              -- utility index which tells us the command we're navigating back to
    scroll = nk_scroll.new(0, 0),  -- the scrollbar position
    isExpanded = true,             -- flag telling us if the console has been expanded or not ( width is increased )
    padding = 5,
    footerHeight = 50,             -- the footer is the section containing the bottom part of the console ( command line + send button )
    logEnabled = false,            -- flag that tells if the log messages should be displayed in the console
    maxMessagesCount = 400,        -- to avoid storing all the messages we can set a limit
    maxHistoryCount = 40,          -- to avoid storing all the commands history we can set a limit
    characterWidth = 7.3,          --hack for a lacking text wrapping support in nuklear
    separateEveryNCharacters = 50, --hack for a lacking text wrapping support in nuklear
}

local function getColorSuccess(isSuccess)
    if isSuccess then
        return successColor
    else
        return failureColor
    end
end

--- Add a new message to the history list
---@param console Console
---@param text string
Console.addHistory = function (console, text)
    table.insert(console.history, text)
    console.historyIndex = #console.history + 1

    local count = #console.history - console.maxHistoryCount
    if count > 0 then
        for _ = 1, count do
            table.remove(console.history, 1)
        end
    end
end

---@param console Console
---@param text string
---@param color table
---@param source string?
Console.addMessage = function (console, text, color, source)
    --  unfortunately Nuklear doesn't support newlines parsing inside label component so we'll manually split each message in chunks
    for s in text:gmatch("[^\r\n]+") do
        table.insert(console.messages, { text = s, col = color, source = source })
    end

    local count = #console.messages - console.maxMessagesCount
    if count > 0 then
        for _ = 1, count do
            table.remove(console.messages, 1)
        end
    end
    console:scrollToEnd()
end

Console.scrollToEnd = function (console)
    --  little hack to put the scrollbar at the bottom whenever we insert a new message
    console.scroll:set(0, #console.messages * 100)
end

--- Send a message from command line to the message list
---@param console Console
Console.send = function (console)
    local text = console.editTB.text
    console:addHistory(text)
    console:addMessage(text, defaultColor)
    local message, isSuccess = CommandManager.execute(text)
    console:addMessage(message, getColorSuccess(isSuccess))
    console.editTB.text = ""
end

--- Move back or forward across the past messages
---@param console Console
---@param step integer
Console.navigateHistory = function (console, step)
    local index = console.historyIndex
    local size = #console.history

    index = index + step
    if index < 1 then
        index = 1
    elseif index > size + 1 then
        index = size + 1
    end

    console.historyIndex = index
    if index < size + 1 then
        console.editTB.text = console.history[index];
    else
        console.editTB.text = ""
    end
end

---@param console Console
---@return string
Console.getText = function (console)
    return console.editTB.text
end

---@param console Console
---@return table
Console.getTextColor = function (console)
    return console.editTB.textColor
end

--- Update the command line text content
---@param console Console
---@param text string
---@param state table
Console.updateText = function (console, text, state)
    local textBox = console.editTB
    textBox.text = text
    textBox.state = state
    if state[nk.EditState.NK_EDIT_COMMITED] and text ~= "" then
        console:send()
    elseif state[nk.EditState.NK_EDIT_INACTIVE] and text == "" then
        textBox.showPlaceholder = true
        textBox.text = textBox.placeholderText
    elseif state[nk.EditState.NK_EDIT_ACTIVE] and textBox.showPlaceholder then
        textBox.text = ""
        textBox.showPlaceholder = false
    end

    textBox.textColor = textBox.show_placeholder and placeHolderColor or editTextColor
end

---
---@param console Console
---@param isWindowHovered boolean
---@param w integer
---@param h integer
---@diagnostic disable-next-line: unused-local
Console.updateWindowSize = function (console, isWindowHovered, w, h)
    local consoleWidth = console.isExpanded and 600 or 400
    if isWindowHovered then
        console.rect.x = console.padding
        console.rect.y = console.padding
        console.rect.w = consoleWidth
        console.rect.h = h - console.padding * 2
    else
        console.rect.x = console.padding
        console.rect.y = h - console.footerHeight - console.padding
        console.rect.w = consoleWidth
        console.rect.h = console.footerHeight
    end

    console.separateEveryNCharacters = (consoleWidth - 8) / console.characterWidth
end

--- Clear all the messages
---@param console Console
Console.clear = function (console)
    console.messages = {}
end

local function getColorByLogLevel(level)
    if level == "trace" then
        return defaultColor
    elseif level == "debug" then
        return debugColor
    elseif level == "info" then
        return defaultColor
    elseif level == "warning" then
        return warningColor
    elseif level == "error" then
        return failureColor
    elseif level == "critical" then
        return criticalColor
    end

    return defaultColor
end

local function onLog(level, message)
    Console:addMessage("[" .. level .. "] " .. message, getColorByLogLevel(level), "log")
end

logListener.register(onLog)

return Console
