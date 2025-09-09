local Game = require "bindings.game"
local CommandUtilities = require "dev.commands.command_utils"
local Utilities = require "utils"

local function showPartyConditions()
    local count = Game.party.getPartySize()
    local message = "Party Conditions\n"
    for i = 1, count do
        local info = Game.party.getCharacterInfo(i, { "condition", "name" })
        local conditionString = ""
        for key, _ in pairs(info.condition) do
            conditionString = conditionString .. enumToString(Game.CharacterCondition, key) .. " "
        end

        if Utilities.isEmpty(conditionString) then
            conditionString = "None"
        end

        message = message .. info.name .. ": " .. conditionString .. "\n"
    end
    return message, true
end

---@param conditionName string
---@param characterIndex integer
---@return string
---@return boolean
local function setCondition(conditionName, characterIndex)
    characterIndex = CommandUtilities.characterOrCurrent(characterIndex)

    Game.party.setCharacterInfo(characterIndex, { condition = stringToEnum(Game.CharacterCondition, conditionName) })
    local info = Game.party.getCharacterInfo(characterIndex, { "name" })

    local message = "Set condition " .. conditionName .. " to " .. info.name
    return message, true
end

local function clearCondition(conditionName, characterIndex)
    characterIndex = CommandUtilities.characterOrCurrent(characterIndex)
    Game.party.clearCondition(characterIndex, stringToEnum(Game.CharacterCondition, conditionName))
    return "Condition " .. conditionName .. " cleared", true
end

local function clearAllConditions()
    local count = Game.party.getPartySize()
    for i = 1, count do
        Game.party.clearCondition(i)
    end
    return "All conditions cleared", true
end

local subCommands = {
    {
        name = "get",
        callback = showPartyConditions,
        description = "Shows the current conditions of all characters in the party."
    },
    {
        name = "set",
        callback = setCondition,
        params = {
            {
                name = "condition",
                type = "enum",
                enumValues = Game.CharacterCondition,
                optional = false,
                description = "Condition to set."
            },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to set condition to." }
        },
        description = "Sets the specified condition for the character."
    },
    {
        name = "clear",
        callback = clearCondition,
        params = {
            {
                name = "condition",
                type = "enum",
                enumValues = Game.CharacterCondition,
                optional = false,
                description = "Condition to clear. If not specified, clears all conditions."
            },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to clear condition from." }
        },
        description = "Clears the specified condition for the specified character."
    },
    {
        name = "clearAll",
        callback = clearAllConditions,
        description = "Clears all conditions on all characters."
    }
}

return {
    name = "condition",
    description = "Change the conditions of the party members.",
    details = "",
    subCommands = subCommands
}
