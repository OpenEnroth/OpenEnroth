local game = require "core.game".bindings
local CommandUtilities = require "dev.commands.command_utils"
local Utilities = require "utils"

local function showPartyConditions()
    local count = game.party.getPartySize()
    local message = "Party Conditions\n"
    for i = 1, count do
        local info = game.party.getCharacterInfo(i, { "condition", "name" })
        local conditionString = ""
        for key, _ in pairs(info.condition) do
            conditionString = conditionString .. Utilities.enumToString(game.CharacterCondition, key) .. " "
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

    game.party.setCharacterInfo(characterIndex, { condition = game.CharacterCondition[conditionName] })
    local info = game.party.getCharacterInfo(characterIndex, { "name" })

    local message = "Set condition " .. conditionName .. " to " .. info.name
    return message, true
end

local function clearCondition(conditionName, characterIndex)
    if conditionName == nil then
        local count = game.party.getPartySize()
        for i = 1, count do
            game.party.clearCondition(i)
        end
        return "All conditions cleared", true
    else
        characterIndex = CommandUtilities.characterOrCurrent(characterIndex)
        game.party.clearCondition(characterIndex, game.CharacterCondition[conditionName])
        return "Condition " .. conditionName .. " cleared", true
    end
end

local subCommands = {
    get = showPartyConditions,
    set = setCondition,
    clear = clearCondition,
    default = showPartyConditions
}

return {
    name = "condition",
    description = "Change the conditions of the party members.",
    details = "",
    callback = subCommands
}
