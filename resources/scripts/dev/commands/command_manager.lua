--- Module that takes care of all the commands being parsed and executed

require "utils"

local command_list = {}

local function split_name_and_params(command_line)
    local firstSpaceIndex = command_line:find(" ")
    if firstSpaceIndex then
        return command_line:sub(1, firstSpaceIndex - 1), command_line:sub(firstSpaceIndex + 1)
    end
    return command_line, ""
end

local function print_command_list(command_name)
    local message = ""
    if command_name == nil then
        for key, command in pairs(command_list) do
            message = message..key..": "..command.description.."\n"
        end
    else
        for key, command in pairs(command_list) do
            if command_name == key then
                message = message..key..": "..command.description.."\n"..command.details.."\n"
            end
        end
    end
    return message, true
end

local help_command = {
    name = "help",
    description = "Show all commands",
    details = "Usage:\n\nhelp [command]",
    callback = print_command_list
}

local command_manager = {
    register = function(command)
        command_list[command.name] = command
    end,

    execute = function(command_line)
        local command_name, params_string = split_name_and_params(command_line)
        local command = command_list[command_name]
        if command == nil then
            local help_message = help_command.callback()
            return command_name.. " is not a valid command. List of valid commands:\n"..help_message, false
        end
        
        local index = 1
        local callback = command.callback
        local params = {}
        if not command.custom_parser then
            params = split_string(params_string, "%s")
            while type(callback) == "table" do
                local par = #params > 0 and params[1] or "default"
                local new_callback = callback[par]
                if new_callback == nil then
                    local message = "Invalid parameter at position "..index.." - Here's a list of valid values:\n"
                    for key, _ in pairs(callback) do
                        if key ~= "default" then
                            message = message..key.."\n"
                        end
                    end
                    return message, false
                else
                    table.remove(params, 1)
                    callback = new_callback
                end
                index = index + 1
            end
        else
            params = command.custom_parser(params_string)
        end

        local message, result = callback(unpack(params))
        if message == nil or result == nil then
            return "The command "..command_name.." does not return a valid message or result. Check the return statements in your script", false
        elseif type(message) ~= "string" then
            return "The command "..command_name.." does not return a valid message. The message must be a string. Current message type: "..type(message), false
        end

        return message, result
    end,
    list = command_list
}

command_manager.register(help_command)

return command_manager
