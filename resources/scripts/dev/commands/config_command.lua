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
        message = param1 .. "." .. param2
    else
        value = Config.getConfig(param1)
        message = param1
    end
    return message .. ": " .. value, true
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
        message = param1 .. "." .. param2
    else
        Config.setConfig(param1, param2)
        value = Config.getConfig(param1)
        message = param1
    end
    return message .. ": " .. tostring(value), true
end

---Toggle the boolean config value
---@param param1 string      - config entry name OR config section name
---@param param2? string      - config entry name if you are passing in section name as the first parameter, nil otherwise
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function toggleConfig(param1, param2)
    local currentValueStr = ""
    local message = ""
    local configInfo = nil
    if param2 ~= nil then
        currentValueStr = Config.getConfig(param1, param2)
        configInfo = Config.listConfigs(param1, param2)
        message = param1 .. "." .. param2
    else
        currentValueStr = Config.getConfig(param1)
        configInfo = Config.listConfigs(param1, "")
        message = param1
    end

    if configInfo[1].type ~= "boolean" then
        return message .. " is not a boolean!", false
    end

    local value = tostring(not Utilities.toBoolean(currentValueStr))
    if param2 ~= nil then
        return setConfig(param1, param2, value)
    else
        return setConfig(param1, value)
    end
end

---Reset the value of a configEntry to its default
---@param param1 string      - config entry name OR config section name
---@param param2? string      - config entry name if you are passing in section name as the first parameter, new value otherwise
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function resetConfig(param1, param2)
    local oldValue = nil
    local newValue = nil
    local message = ""
    if param2 ~= nil then
        oldValue = Config.getConfig(param1, param2)
        newValue = Config.resetConfig(param1, param2)
        message = param1 .. "." .. param2
    else
        oldValue = Config.getConfig(param1)
        newValue = Config.resetConfig(param1)
        message = param1
    end
    return message .. ": " .. tostring(oldValue) .. " -> " .. tostring(newValue), true
end

---List all matching configEntries
---@param param1? string     - partial name or value of the config entry OR section of the config entry
---@param param2? string    - partial name or value of the config entry if you are passing in section name as the first parameter
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if any entries were found
local function listConfigs(param1, param2)
    local message = ""
    if not param1 then
        param1 = ""
    end
    if not param2 then
        param2 = ""
    end
    local matches = Config.listConfigs(param1, param2)
    local count = #matches
    if count == 0 then
        return "No matches", false
    elseif count == 1 then
        local config = matches[1]
        message = message .. "section: " .. config.section .. "\n"
        message = message .. "name: " .. config.name .. "\n"
        message = message .. "type: " .. config.type .. "\n"
        message = message .. "description: " .. config.description .. "\n"
        message = message .. "default: " .. config.default .. "\n"
        message = message .. "value: " .. config.value .. "\n"
    else
        local sectionHeader = nil
        for _, config in pairs(matches) do
            if (config.section ~= sectionHeader) then
                sectionHeader = config.section
                message = message .. "[" .. sectionHeader .. "]\n"
            end
            message = message .. "  " .. config.name .. ": " .. config.value .. "\n"
        end
    end
    return message, true
end

local subCommands = {
    get = getConfig,
    set = setConfig,
    toggle = toggleConfig,
    reset = resetConfig,
    list = listConfigs,
}

return {
    name = "config",
    description = "Show or change any game config value.",
    details = [[
'get', 'set', 'reset' and 'toggle' require an exact name of a
    config entry, optionally prefixed by the section name.
    When the section is omitted and the entry name exists in
    several sections, the choice is by internal ordering.
'set' requires an additional parameter for the value,
    which MUST be convertible to the appropriate type.
'list' accepts an optional filter prefixed by an optional section
       name. Specifying a section limits the filter (which can
       still be omitted) to that section, otherwise the filter
       is applied to all settings.
    The filter can can be:
    - missing to list an entire section / all settings,
    - an exact value (quoting string or enum values)
      to list all settings having that value,
    - 'default' or '!default'
      to filter by settings that are at their default or not,
    - or a partial config entry name (unquoted)
      to list all matching settings.
    All matches are listed by section headers,
        unless the match is unique, in which case
        all attributes of the entry are listed.
]],
    callback = subCommands
}
