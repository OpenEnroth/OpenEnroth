local Config = require "bindings.config"
local CommandUtilities = require "dev.commands.command_utils"

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

-- Build the list of available sections as if they were enums value
--- @type table<string, integer>
local configSectionsEnum = {}
--- @type table<string, table<string, integer>>
local configEntriesEnum = {}

local matches = Config.list("", "")

local sectionIndexValue = 1
local entryIndexValue = 1
for _, entry in pairs(matches) do
    if configSectionsEnum[entry.section] == nil then
        configSectionsEnum[entry.section] = sectionIndexValue
        configEntriesEnum[entry.section] = {}
        sectionIndexValue = sectionIndexValue + 1
    end

    configEntriesEnum[entry.section][entry.name] = entryIndexValue
    entryIndexValue = entryIndexValue + 1
end

--- @param dataParam DataParameter
--- @param infoParam CommandParameter
--- @param allDataParams table<string, DataParameter>
--- @return boolean
local function renderConfigValue(dataParam, infoParam, allDataParams)
    if allDataParams.section.value == nil or allDataParams.entry.value == nil then
        return false
    end

    --- @type string
    local sectionName = allDataParams.section.value
    --- @type string
    local entryName = allDataParams.entry.value
    local entry = Config.entry(sectionName, entryName)
    if dataParam.value == nil then
        dataParam.value = tostring(entry.value)
    end
    return CommandUtilities.defaultParamRenderer(entry:getType(), dataParam, infoParam, allDataParams)
end

local subCommands = {
    {
        name = "get",
        callback = getConfig,
        params = {
            { name = "section", type = "enum", enumValues = configSectionsEnum },
            {
                name = "entry",
                type = "enum",
                enumValues = function (params)
                    return configEntriesEnum[params.section.value]
                end,
            }
        },
        description = "Get the value of a config entry."
    },
    {
        name = "set",
        callback = setConfig,
        params = {
            { name = "section", type = "enum", enumValues = configSectionsEnum, description = "Name of the config entry to set." },
            {
                name = "entry",
                type = "enum",
                enumValues = function (params)
                    return configEntriesEnum[params.section.value]
                end,
                description = "Name of the config entry to set."
            },
            {
                name = "value",
                type = "string",
                description = "New value to set for the config entry.",
                resetOnOtherParamChange = function () return nil end,
                renderer = renderConfigValue
            }
        },
        description = "Set the value of a config entry."
    },
    {
        name = "reset",
        callback = resetConfig,
        params = {
            { name = "section", type = "enum", enumValues = configSectionsEnum },
            {
                name = "entry",
                type = "enum",
                enumValues = function (params)
                    return configEntriesEnum[params.section.value]
                end,
            }
        },
        description = "Reset the value of a config entry to its default."
    },
    {
        name = "toggle",
        callback = toggleConfig,
        params = {
            { name = "section", type = "enum", enumValues = configSectionsEnum },
            {
                name = "entry",
                type = "enum",
                enumValues = function (params)
                    return configEntriesEnum[params.section.value]
                end,
            }
        },
        description = "Toggle the boolean value of a config entry."
    },
    {
        name = "list",
        callback = listConfigs,
        params = {
            { name = "section", type = "enum", enumValues = configSectionsEnum },
            {
                name = "entry",
                type = "enum",
                enumValues = function (params)
                    return configEntriesEnum[params.section.value]
                end,
            }
        },
        description = "Show config entry."
    }
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
    subCommands = subCommands
}
