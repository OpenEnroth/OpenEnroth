local Utilities = require "dev.commands.command_utils"
local game = require "core.game".bindings

local function showCharsHp()
    local count = game.party.getPartySize()
    local message = ""
    for i = 1, count do
        local info = game.party.getCharacterInfo(i, { "name", "hp", "maxHp" })
        message = message .. info.name .. ": " .. info.hp .. "/" .. info.maxHp .. "\n"
    end
    return message, true
end

local function fullHeal(charIndex)
    charIndex = Utilities.characterOrCurrent(charIndex)

    local info = game.party.getCharacterInfo(charIndex, { "name", "maxHp", "maxMana" })
    game.party.clearCondition(charIndex)
    game.party.setCharacterInfo(charIndex, {
        hp = info.maxHp,
        mana = info.maxMana
    })
    return info.name .. "'s hp and mana are restored.", true
end

local subCommands = {
    get = showCharsHp,
    rem = Utilities.changeCharProperty("hp", Utilities.opType.rem, false, tonumber),
    add = Utilities.changeCharProperty("hp", Utilities.opType.add, false, tonumber),
    set = Utilities.changeCharProperty("hp", Utilities.opType.set, false, tonumber),
    full_heal = fullHeal,
    default = showCharsHp
}

return {
    name = "hp",
    description = "Show and manipulate characters hp.",
    details = "",
    callback = subCommands
}
