local Config = require "bindings.config"
local Utilities = require "utils"

---Get the value of a configEntry
---@param param1 string         - config entry name OR config section name
---@param param2? string        - config entry name if you are passing in section name as the first parameter
---@return string               - the message sent back to the console
---@return boolean              - flag that tells if the command has been successful
local function getConfig(param1, param2)
    local value = nil
    local message = ""
    if param2 ~= nil then
        value = Config.getConfig(param1, param2)
        message = param1 .. "." .. param2 .. ": "
    else
        value = Config.getConfig(param1)
        message = param1 .. ": "
    end
    return message .. tostring(value), true
end

---Change the value of the configEntry
---@param param1 string      - config entry name OR config section name
---@param param2 string      - config entry name if you are passing in section name as the first parameter, new value otherwise
---@param param3? string     - new value if you are passing in section name as the first parameter, nil otherwise
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function setConfig(param1, param2, param3)
    local value = nil
    local message = ""
    if param3 ~= nil then
        Config.setConfig(param1, param2, param3)
        value = Config.getConfig(param1, param2)
        message = param1 .. "." .. param2 .. ": "
    else
        Config.setConfig(param1, param2)
        value = Config.getConfig(param1)
        message = param1 .. ": "
    end
    return message .. tostring(value), true
end

---Toggle the boolean config value
---@param entry string       - name of the config entry
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function toggleConfig(entry)
    local currentValueStr = Config.getConfig(entry)
    local value = Utilities.toBoolean(currentValueStr)
    return setConfig(entry, tostring(not value))
end

local subCommands = {
    get = getConfig,
    set = setConfig,
    toggle = toggleConfig,
}

return {
    name = "config",
    description = "Change any game config value.",
    details = "",
    callback = subCommands
}
