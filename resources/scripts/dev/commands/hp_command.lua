local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local function showCharsHp()
    local count = MM.party.getPartySize()
    local message = ""
    for i = 1, count do
        local info = MM.party.getCharacterInfo(i, { "name", "hp", "max_hp" })
        message = message .. info.name .. ": " .. info.hp .. "/" .. info.max_hp .. "\n"
    end
    return message, true
end

local function fullHeal(charIndex)
    charIndex = Utilities.characterOrCurrent(charIndex)

    local info = MM.party.getCharacterInfo(charIndex, { "name", "max_hp", "max_mana" })
    MM.party.clearCondition(charIndex)
    MM.party.setCharacterInfo(charIndex, {
        hp = info.max_hp,
        mana = info.max_mana
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
