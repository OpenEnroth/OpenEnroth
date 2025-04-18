local Config = require "bindings.config"

---Get the value of a configEntry
---@param param1 string         - config entry name OR config section name
---@param param2? string        - config entry name if you are passing in section name as the first parameter
---@return string               - the message sent back to the console
---@return boolean              - flag that tells if the command has been successful
local function getConfig(param1, param2)
    local entry = nil
    if param2 == nil then
        entry = Config.entry(param1)
    else
        entry = Config.entry(param1, param2)
    end
    return entry.path .. ": " .. entry.value, true
end

---Change the value of the configEntry
---@param param1 string      - config entry name OR config section name
---@param param2 string      - config entry name if you are passing in section name as the first parameter, new value otherwise
---@param param3? string     - new value if you are passing in section name as the first parameter, nil otherwise
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function setConfig(param1, param2, param3)
    local entry = nil
    local value = ""
    if param3 == nil then
        entry = Config.entry(param1)
        value = param2
    else
        entry = Config.entry(param1, param2)
        value = param3
    end
    entry.value = value
    return entry.path .. ": " .. entry.value, true
end

---Reset the value of a configEntry to its default
---@param param1 string      - config entry name OR config section name
---@param param2? string     - config entry name if you are passing in section name as the first parameter, nil otherwise
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function resetConfig(param1, param2)
    local entry = nil
    if param2 == nil then
        entry = Config.entry(param1)
    else
        entry = Config.entry(param1, param2)
    end
    entry:reset()
    return entry.path .. ": " .. entry.value, true
end

---Toggle the boolean config value
---@param param1 string      - config entry name OR config section name
---@param param2? string     - config entry name if you are passing in section name as the first parameter, nil otherwise
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function toggleConfig(param1, param2)
    local entry = nil
    if param2 == nil then
        entry = Config.entry(param1)
    else
        entry = Config.entry(param1, param2)
    end
    entry:toggle()
    return entry.path .. ": " .. entry.value, true
end

---List all matching configEntries
---@param param1? string     - partial name or value of the config entry OR section of the config entry
---@param param2? string     - partial name or value of the config entry if you are passing in section name as the first parameter
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if any entries were found
local function listConfigs(param1, param2)
    local matches = nil
    if param1 == nil then
        matches = Config.list("", "")
    elseif param2 == nil then
        matches = Config.list("", param1)
    else
        matches = Config.list(param1, param2)
    end

    local count = #matches
    local message = ""
    if count == 0 then
        return "No matches", false
    elseif count == 1 then
        local entry = matches[1]
        message = message .. "section: " .. entry.section .. "\n"
        message = message .. "name: " .. entry.name .. "\n"
        message = message .. "description: " .. entry.description .. "\n"
        message = message .. "default: " .. entry.default .. "\n"
        message = message .. "value: " .. entry.value .. "\n"
    else
        local sectionHeader = nil
        for _, entry in pairs(matches) do
            if (entry.section ~= sectionHeader) then
                sectionHeader = entry.section
                message = message .. "[" .. sectionHeader .. "]\n"
            end
            message = message .. "  " .. entry.name .. ": " .. entry.value .. "\n"
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
