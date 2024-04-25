local config = require "core.config".bindings

---Get the value of a configEntry
---@param configName string    - config entry name
---@param sectionName? string  - section of the config entry
---@return string               - the message sent back to the console
---@return boolean              - flag that tells if the command has been successful
local function getConfig(configName, sectionName)
    local value = nil
    local message = ""
    if sectionName ~= nil then
        value = config.getConfig(sectionName, configName)
        message = "[" .. sectionName .. "] - " .. configName .. ": "
    else
        value = config.getConfig(configName)
        message = configName .. ": "
    end
    return message .. tostring(value), true
end

---Change the value of the configEntry
---@param configName string - name of the configEntry
---@param param2 string      - section of the config entry. If param3 is nil param2 represents the value instead
---@param param3? string     - new value to apply
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function setConfig(configName, param2, param3)
    local value = nil
    local message = ""
    local sectionName = nil
    if param3 ~= nil then
        sectionName = param2
        value = param3
    else
        value = param2
    end

    if sectionName ~= nil then
        config.setConfig(sectionName, configName, value)
        value = config.getConfig(sectionName, configName)
        message = "[" .. sectionName .. "] - " .. configName .. ": "
    else
        config.setConfig(configName, value)
        value = config.getConfig(configName)
        message = configName .. ": "
    end
    return message .. tostring(value), true
end

local subCommands = {
    get = getConfig,
    set = setConfig,
}

return {
    name = "config",
    description = "Change any game config value.",
    details = "",
    callback = subCommands
}
