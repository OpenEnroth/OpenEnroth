local MM = require "mmbindings"

---@class CommandUtilities
local CommandUtilities = {}

---@enum opType
CommandUtilities.opType = {
    set = 1,
    rem = 2,
    add = 3
}

--- Utility factory-function that generate a new function used as command to update one property/stat for a character
--- Example that generate a command used to add experience points to a character:
---     changeCharProperty("xp", opType.add, true)
---
---@param key string            - field referring to the CharacterInfo table ( "xp", "age", "sp", etc... )
---@param op opType            - the operation to execute on the variable
---@param playAward boolean     - play the award effect after the operation has been executed
---@param conversion fun(val:any):any   - function used to convert the input value to another type
---@return fun(val: string, charIndex: integer): string, boolean
CommandUtilities.changeCharProperty = function (key, op, playAward, conversion)
    ---@param value any
    ---@param charIndex? integer
    ---@return string, boolean
    return function (value, charIndex)
        if conversion then
            value = conversion(value)
        end

        charIndex = CommandUtilities.characterOrCurrent(charIndex)

        local get = MM.party.getCharacterInfo
        local set = MM.party.setCharacterInfo
        value = value ~= nil and value or 0
        local info = get(charIndex, { key, "name" })
        local characterName = info.name
        ---@type table<string,any>
        local newData = {}
        local message = ""
        if op == CommandUtilities.opType.set then
            newData[key] = value
            set(charIndex, newData)
            message = "Set " .. value .. " " .. key .. " for " .. characterName
        elseif op == CommandUtilities.opType.rem then
            newData[key] = info[key] - value
            set(charIndex, newData)
            message = characterName .. " lost " .. value .. " " .. key ..
                ". Current " .. key .. ": " .. tostring(get(charIndex, { key })[key])
        elseif op == CommandUtilities.opType.add then
            newData[key] = info[key] + value
            set(charIndex, newData)
            message = characterName .. " gained " .. value .. " " ..
                key .. ". Current " .. key .. ": " .. tostring(get(charIndex, { key })[key])
        end

        if playAward then
            MM.party.playCharacterAwardSound(charIndex)
        end
        return message, true
    end
end

--- Utility factory-function that generate a new function used as command to show one property/stat for each member of the party
--- Example that generate a command that shows the skils points of each character:
---     showCharsProperty("sp")
---
---@param key string field referring to the character_info table
---@param serializer? fun(value:any):string) function that convert the value to string
---@return function
CommandUtilities.showCharsProperty = function (key, serializer)
    return function ()
        local count = MM.party.getPartySize()
        local message = "Party " .. key .. "\n"
        for i = 1, count do
            local info = MM.party.getCharacterInfo(i, { key, "name" })
            local value = serializer and serializer(info[key]) or info[key]
            message = message .. info.name .. ": " .. value .. "\n"
        end
        return message, true
    end
end

--- Utility factory-function that generate a new function used as command to update a value retrieved by callbacks
--- Example of a command that remove golds from the party:
---     changeProperty(MM.party.getGold, MM.party.setGold, ACTION_TYPE.rem, "gold")
---
---@param get function      - getter function used to retrieve the current value
---@param set function      - setter function used to update the current value
---@param op opType        - the operation to execute on the variable
---@param propName string  - name of the property. Used only for prompting, it's not used to retrieve the value
---@param conversion? fun(val:any):any
---@param serializer? fun(val:any):string
---@return function
CommandUtilities.changeProperty = function (get, set, op, propName, conversion, serializer)
    return function (value)
        if conversion then
            value = conversion(value)
        end

        local message = ""
        if op == CommandUtilities.opType.set then
            set(value)
            local serializedValue = serializer and serializer(value) or value;
            message = "Set " .. serializedValue .. " " .. propName
        elseif op == CommandUtilities.opType.add then
            ---@type integer
            local total = get() + value
            set(total)
            message = "Added " .. tostring(value) .. " " .. propName .. ". Current: " .. tostring(get())
        elseif op == CommandUtilities.opType.rem then
            ---@type integer
            local total = get() - value
            set(total)
            message = "Removed " .. tostring(value) .. " " .. propName .. ". Current: " .. tostring(get())
        end
        return message, true
    end
end

--- Utility factory-function that generate a new function used as command to show a value
--- Example of a function generating a command showing the current alignment:
---     showProperty(MM.party.getAlignment, "alignment")
---@param get function      getter function used to retrieve the current value
---@param propName string  name of the property. Used only for prompting, it's not used to retrieve the value
---@return function
---@param serializer? fun(val:any):string
CommandUtilities.showProperty = function (get, propName, serializer)
    return function ()
        local value = serializer and serializer(get()) or get()
        return "Current " .. propName .. ": " .. value, true
    end
end

--- Provide a valid character index. If the input is nil it returns the active character in the party
---@param charIndex? integer
---@return integer
CommandUtilities.characterOrCurrent = function (charIndex)
    if charIndex == nil then
        return MM.party.getActiveCharacter()
    end
    return charIndex
end

return CommandUtilities
