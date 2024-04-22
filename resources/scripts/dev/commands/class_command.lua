local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local function becomeLich(charIndex)
    charIndex = Utilities.characterOrCurrent(charIndex)
    local info = MM.party.getCharacterInfo(charIndex, { "name", "class" })
    if info.class == MM.ClassType.Lich then
        return info.name .. " is already a Lich!!", false
    end

    MM.party.setCharacterInfo(charIndex, {
        class = MM.ClassType.Lich
    })

    local lichJar = {
        id = MM.ItemType.LichJarFull,
        holder = tonumber(charIndex)
    }
    MM.party.addCustomItemToInventory(charIndex, lichJar)
    return info.name .. " became a Lich.", true
end

local subCommands = {
    lich = becomeLich
}

return {
    name = "class",
    description = "Change and get information about character class",
    callback = subCommands
}
