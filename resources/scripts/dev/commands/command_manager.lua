--- Module that takes care of all the commands being parsed and executed

require "utils"

local command_list = {}

local parse_command_line = function(command_line)
    local command_name = ""
    local params = {}
    local i = 1
    local separator = "%s"
    for str in string.gmatch(command_line, "([^"..separator.."]+)") do
        if i == 1 then
            command_name = str
        else
            table.insert(params, str)
        end
        i = i + 1
    end

    return command_name, params
end

local help_command = function(command_name)
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

local command_manager = {
    register = function(name, description, details, callback)
        command_list[name] = {
            callback = callback,
            description = description,
            details = details
        }
    end,

    execute = function(command_line)
        local command_name, params = parse_command_line(command_line)
        local command = command_list[command_name]
        if command == nil then
            local help_message = help_command()
            return command_name.. " is not a valid command. List of valid commands:\n"..help_message, false
        end
        
        local index = 1
        local callback = command.callback;
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

        local message, result = callback(unpack(params))
        if message == nil or result == nil then
            return "The command "..command_name.." does not return a valid message or result. Check the return statements in your script", false
        end

        return message, result
    end,
    list = command_list
}

command_manager.register("help", "Show all commands", "Usage:\n\nhelp [command]", help_command)

return command_manager
