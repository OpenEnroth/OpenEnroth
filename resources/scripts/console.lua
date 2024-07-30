--- Module that takes care of the state of the debug console

local CommandManager = require "dev.commands.command_manager"
local LogListener = require "core.log_listener"
local Utilities = require "utils"

local successColor = Utilities.color(0.25, 0.57, 0.87, 1)
local defaultColor = Utilities.color(0.63, 0.63, 0.63, 1)
local debugColor = Utilities.color(1.00, 1.00, 1.00, 0.50)
local warningColor = Utilities.color(1.00, 1.00, 0.00, 1.00)
local failureColor = Utilities.color(1.00, 0.00, 0.00, 1.00)
local criticalColor = Utilities.color(1.00, 0.50, 0.00, 1.00)

---@class ConsoleMessage
---@field text string
---@field col table Color to use for the text
---@field source string Tell from where the message has been sent

---@class Console The console table contains the state of the console. From position to the number of messages
---@field messages table<integer, ConsoleMessage> List of all the messages displayed in the console
---@field maxMessagesCount integer Maximum number of messages the console can show. After that the oldest messages are removed
---@field history table<integer, string> List of all the messages sent by the user
---@field historyIndex integer current message being shown ( used to go back/forward when the user want to execute send messages )
---@field logEnabled boolean
---@field autoMinimize boolean
---@field text string
local Console = {
    rect = {},
    text = "",              -- current command being typed
    messages = {},          -- each message being sent to the console is stored in this table
    history = {},           -- the history of commands being executed. Useful to navigate back to previously written commands
    historyIndex = 1,       -- utility index which tells us the command we're navigating back to
    logEnabled = false,     -- flag that tells if the log messages should be displayed in the console
    autoMinimize = true,    -- Minimize console when the mouse is not hover it
    maxMessagesCount = 400, -- to avoid storing all the messages we can set a limit
    maxHistoryCount = 40,   -- to avoid storing all the commands history we can set a limit
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
    table.insert(console.messages, { text = text, col = color, source = source })

    local count = #console.messages - console.maxMessagesCount
    if count > 0 then
        for _ = 1, count do
            table.remove(console.messages, 1)
        end
    end
end

--- Send a message from command line to the message list
---@param console Console
---@param text? string If text is nil the current commandline text is used instead
Console.send = function (console, text)
    if not text then
        text = console.text
    end

    console:addHistory(text)
    console:addMessage(text, defaultColor)
    local message, isSuccess = CommandManager.execute(text)
    console:addMessage(message, getColorSuccess(isSuccess))
    console.text = ""
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
        console.text = console.history[index];
    else
        console.text = ""
    end
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

LogListener.register(onLog)

return Console
