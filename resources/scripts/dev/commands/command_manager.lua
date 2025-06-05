--- Module that takes care of all the commands being parsed and executed

local Utilities = require "utils"

---@alias CommandFunction fun(parameter...):string, boolean

---@class CommandParameter
---@field name string
---@field type string
---@field optional? boolean
---@field description string
---@field enumValues? table<string, any>  -- Used for enum types, contains the possible values for the parameter
---@field defaultValue? fun():string
---@field renderer? fun(dataParam: DataParameter, infoParam: CommandParameter, allDataParams: table<string, DataParameter>):boolean
---@field resetOnOtherParamChange? fun():string|nil

---@class Command
---@field name string
---@field description string
---@field details string
---@field callback? CommandFunction
---@field subCommands? table<integer, Command>
---@field params? table<integer, CommandParameter>

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
        local finalCommand = command

        local commandParameters = Utilities.splitString(paramsString, "%s")
        while finalCommand.callback == nil do
            local subCommandName = #commandParameters > 0 and commandParameters[1] or "default"

            --- @type Command
            local subCommand = Utilities.findIf(
                finalCommand.subCommands,
                function (item)
                    return item.name == subCommandName
                end
            )

            if subCommand == nil then
                local message = "Invalid parameter '" ..
                    subCommandName .. "' at position " .. index .. " - Here's a list of valid values:\n"
                for _, item in ipairs(finalCommand.subCommands) do
                    if item.name ~= "default" then
                        message = message .. item.name .. "\n"
                    end
                end
                return message, false
            else
                finalCommand = subCommand
                table.remove(commandParameters, 1)
            end
            index = index + 1
        end

        local message, result = "", false
        ---@type boolean, string|nil
        local status, err = pcall(
            function ()
                ---let's convert the params to the expected types, if we don't do this the pcall is going to fail giving us errors similar to "cannot convert string to number"
                ---@type table<integer, any>
                local finalParams = {}
                if finalCommand.params ~= nil then
                    for i, param in ipairs(commandParameters) do
                        if finalCommand.params[i] then
                            local paramType = finalCommand.params[i].type
                            if paramType == "number" then
                                table.insert(finalParams, tonumber(param))
                            elseif paramType == "boolean" then
                                table.insert(finalParams, Utilities.toBoolean(param))
                            elseif paramType == "characterIndex" then
                                table.insert(finalParams, tonumber(param))
                            else
                                table.insert(finalParams, param)
                            end
                        end
                    end
                end

                ---TODO(Gerark) unpack is deprecated in 5.4 but need to check why the alternative is not working at runtime.
                --- Probably we're running on an older version. If that's the case we should align the linter version
                ---@diagnostic disable-next-line: deprecated
                message, result = finalCommand.callback(unpack(finalParams))
            end
        )
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
                " does not return a valid message. The message must be a string. Current message type: " ..
                tostring(type(message))
        end

        return message, result
    end,
    list = commandList
}

CommandManager.register(helpCommand)

return CommandManager
