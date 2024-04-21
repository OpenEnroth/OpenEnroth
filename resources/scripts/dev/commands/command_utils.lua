local mm = require "mmbindings"

local command_utilities = {}

---@enum op_type
command_utilities.OP_TYPE = {
	set = 1,
	rem = 2,
	add = 3
}

--- Utility factory-function that generate a new function used as command to update one property/stat for a character
--- Example that generate a command used to add experience points to a character:
---     change_char_property("xp", ACTION_TYPE.add, true)
---
---@param key string            - field referring to the character_info table ( "xp", "age", "sp", etc... )
---@param op op_type            - the operation to execute on the variable
---@param play_award boolean    - play the award effect after the operation has been executed
---@return function
command_utilities.change_char_property = function(key, op, play_award, conversion)
    return function(value, char_index)
        if conversion then
            value = conversion(value)
        end

        char_index = command_utilities.character_or_current(char_index)

        local get = mm.party.get_character_info
        local set = mm.party.set_character_info
        value = value ~= nil and value or 0
        local info = get(char_index, { key, "name" })
        local newData = {}
        local message = ""
        if op == command_utilities.OP_TYPE.set then
            newData[key] = value
            set(char_index, newData)
            message = "Set "..value.." "..key.." for "..info.name
        elseif op == command_utilities.OP_TYPE.rem then
            newData[key] = info[key] - value
            set(char_index, newData)
            message = info.name.." lost "..value.." "..key..". Current "..key..": "..get(char_index, { key })[key]
        elseif op == command_utilities.OP_TYPE.add then
            newData[key] = info[key] + value
            set(char_index, newData)
            message = info.name.." gained "..value.." "..key..". Current "..key..": "..get(char_index, { key })[key]
        end

        if play_award then
            mm.party.play_character_award_sound(char_index)
        end
        return message, true
    end
end

--- Utility factory-function that generate a new function used as command to show one property/stat for each member of the party
--- Example that generate a command that shows the skils points of each character:
---     show_chars_property("sp")
---
---@param key string field referring to the character_info table
---@return function
command_utilities.show_chars_property = function(key, serializer)
    return function()
        local count = mm.party.get_party_size()
        local message = "Party "..key.."\n"
        for i = 1, count do
            local info = mm.party.get_character_info(i, { key, "name" })
            local value = serializer and serializer(info[key]) or info[key]
            message = message..info.name..": "..value.."\n"
        end
        return message, true
    end
end

--- Utility factory-function that generate a new function used as command to update a value retrieved by callbacks
--- Example of a command that remove golds from the party:
---     change_property(mm.party.get_gold, mm.party.set_gold, ACTION_TYPE.rem, "gold")
---
---@param get function      - getter function used to retrieve the current value
---@param set function      - setter function used to update the current value
---@param op op_type        - the operation to execute on the variable
---@param prop_name string  - name of the property. Used only for prompting, it's not used to retrieve the value
---@return function
command_utilities.change_property = function(get, set, op, prop_name, conversion, serializer)
    return function(value)
        if conversion then
            value = conversion(value)
        end

        local message = ""
        if op == command_utilities.OP_TYPE.set then
            set(value)
            local serializedValue = serializer and serializer(value) or value;
            message = "Set "..serializedValue.." "..prop_name
        elseif op == command_utilities.OP_TYPE.add then
            local total = get() + value
            set(total)
            message = "Added "..value.." "..prop_name..". Current: "..get()
        elseif op == command_utilities.OP_TYPE.rem then
            local total = get() - value
            set(total)
            message = "Removed "..value.." "..prop_name..". Current: "..get()
        end
        return message, true
    end
end

--- Utility factory-function that generate a new function used as command to show a value
--- Example of a function generating a command showing the current alignment:
---     show_property(mm.party.get_alignment, "alignment")
---@param get function      getter function used to retrieve the current value
---@param prop_name string  name of the property. Used only for prompting, it's not used to retrieve the value
---@return function
command_utilities.show_property = function(get, prop_name, serializer)
    return function()
        local value = serializer and serializer(get()) or get()
        return "Current "..prop_name..": "..value, true
    end
end

--- Provide a valid character index. If the input is nil it returns the active character in the party
---@param char_index integer
---@return integer
command_utilities.character_or_current = function(char_index)
    if char_index == nil then
        return mm.party.get_active_character()
    end
    return char_index
end

return command_utilities
