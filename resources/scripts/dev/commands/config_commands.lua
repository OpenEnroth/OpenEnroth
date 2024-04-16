---Get the value of a configEntry
---@param config_name string    - config entry name
---@param section_name? string  - section of the config entry 
---@return string               - the message sent back to the console
---@return boolean              - flag that tells if the command has been successful
local function get_config(config_name, section_name)
    local value
    local message
    if section_name ~= nil then
        value = dev.config_get(section_name, config_name)
        message = "["..section_name.."] - "..config_name..": "
    else
        value = dev.config_get(config_name)
        message = config_name..": "
    end
    return message..tostring(value), true
end

---Change the value of the configEntry
---@param config_name string - name of the configEntry
---@param param2 string      - section of the config entry. If param3 is nil param2 represents the value instead 
---@param param3? string     - new value to apply
---@return string            - the message sent back to the console
---@return boolean           - flag that tells if the command has been successful
local function set_config(config_name, param2, param3)
    local value
    local message
    local section_name
    if param3 ~= nil then
        section_name = param2
        value = param3
    else
        value = param2
    end

    if section_name ~= nil then
        dev.config_set(section_name, config_name, value)
        value = dev.config_get(section_name, config_name)
        message = "["..section_name.."] - "..config_name..": "
    else
        dev.config_set(config_name, value)
        value = dev.config_get(config_name)
        message = config_name..": "
    end
    return message..tostring(value), true
end

local config_commands = {
    get = get_config,
    set = set_config,
}

return config_commands