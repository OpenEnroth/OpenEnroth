local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

local function showCharsHp()
    local count = Game.party.getPartySize()
    local message = ""
    for i = 1, count do
        local info = Game.party.getCharacterInfo(i, { "name", "hp", "maxHp" })
        message = message .. info.name .. ": " .. info.hp .. "/" .. info.maxHp .. "\n"
    end
    return message, true
end

local function fullHeal(charIndex)
    charIndex = Utilities.characterOrCurrent(charIndex)

    local info = Game.party.getCharacterInfo(charIndex, { "name", "maxHp", "maxMana" })
    Game.party.clearCondition(charIndex)
    Game.party.setCharacterInfo(charIndex, {
        hp = info.maxHp,
        mana = info.maxMana
    })
    return info.name .. "'s hp and mana are restored.", true
end

local subCommands = {
    {
        name = "get",
        callback = showCharsHp,
        description = "Shows the current hp of all characters in the party."
    },
    {
        name = "add",
        callback = Utilities.changeCharProperty("hp", Utilities.opType.add, false, tonumber),
        params = {
            { name = "hp",   type = "number",         optional = false, description = "Amount of hp to add." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to add hp to." }
        },
        description = "Adds the specified amount of hp to the character."
    },
    {
        name = "rem",
        callback = Utilities.changeCharProperty("hp", Utilities.opType.rem, false, tonumber),
        params = {
            { name = "hp",   type = "number",         optional = false, description = "Amount of hp to remove." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to remove hp from." }
        },
        description = "Removes the specified amount of hp from the character."
    },
    {
        name = "set",
        callback = Utilities.changeCharProperty("hp", Utilities.opType.set, false, tonumber),
        params = {
            { name = "hp",   type = "number",         optional = false, description = "Amount of hp to set." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to set hp to." }
        },
        description = "Sets the character's hp to the specified value."
    },
    {
        name = "full_heal",
        callback = fullHeal,
        params = {
            { name = "char", type = "characterIndex", optional = false, description = "Character index to fully heal." }
        },
        description = "Fully heals the character, restoring hp and mana."
    },
}

return {
    name = "hp",
    description = "Show and manipulate characters hp.",
    details = "",
    subCommands = subCommands
}
