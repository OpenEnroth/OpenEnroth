--- Module that takes care of all the commands being parsed and executed

local Utilities = require "utils"

---@alias CommandFunction fun(parameter...):string, boolean

---@class Command
---@field name string
---@field description string
---@field details string
---@field callback CommandFunction | table<string, CommandFunction>
---@field customParser? fun(commandLine:string): table<string>

---@type table<string, Command>
local commandList = {}

---@param commandLine string
---@return string
---@return string
local function splitNameAndParams(commandLine)
    local firstSpaceIndex = commandLine:find(" ")
    if firstSpaceIndex then
        return commandLine:sub(1, firstSpaceIndex - 1), commandLine:sub(firstSpaceIndex + 1)
    end
    return commandLine, ""
end

local function printCommandList(commandName)
    local message = ""
    if commandName == nil then
        for key, command in pairs(commandList) do
            message = message .. key .. ": " .. command.description .. "\n"
        end
    else
        for key, command in pairs(commandList) do
            if commandName == key then
                message = message .. key .. ": " .. command.description .. "\n" .. command.details .. "\n"
            end
        end
    end
    return message, true
end

local helpCommand = {
    name = "help",
    description = "Show all commands",
    details = "Usage:\n\nhelp [command]",
    callback = printCommandList
}

local CommandManager = {
    register = function (command)
        commandList[command.name] = command
    end,

    execute = function (commandLine)
        local commandName, paramsString = splitNameAndParams(commandLine)
        local command = commandList[commandName]
        if command == nil then
            local helpMessage = helpCommand.callback()
            return commandName .. " is not a valid command. List of valid commands:\n" .. helpMessage, false
        end

        local index = 1
        local callback = command.callback
        local params = {}
        if not command.customParser then
            params = Utilities.splitString(paramsString, "%s")
            while type(callback) == "table" do
                local par = #params > 0 and params[1] or "default"
                local newCallback = callback[par]
                if newCallback == nil then
                    local message = "Invalid parameter '" .. par .. "' at position " .. index .. " - Here's a list of valid values:\n"
                    for key, _ in pairs(callback) do
                        if key ~= "default" then
                            message = message .. key .. "\n"
                        end
                    end
                    return message, false
                else
                    table.remove(params, 1)
                    callback = newCallback
                end
                index = index + 1
            end
        else
            params = command.customParser(paramsString)
        end

        local message, result = "", false
        ---TODO(Gerark) unpack is deprecated in 5.4 but need to check why the alternative is not working at runtime.
        --- Probably we're running on an older version. If that's the case we should align the linter version
        ---@diagnostic disable-next-line: deprecated
        local status, err = pcall(function () message, result = callback(unpack(params)) end)
        if not status then
            result = false
            message = "The command " ..
                commandName .. " has failed miserably. " .. err
        elseif message == nil or result == nil then
            result = false
            message = "The command " ..
                commandName .. " does not return a valid message or result. Check the return statements in your script"
        elseif type(message) ~= "string" then
            result = false
            message = "The command " ..
                commandName ..
                " does not return a valid message. The message must be a string. Current message type: " .. type(message)
        end

        return message, result
    end,
    list = commandList
}

CommandManager.register(helpCommand)

return CommandManager
