local Utilities = require "dev.commands.command_utils"
local game = requireGame()

local function becomeLich(charIndex)
    charIndex = Utilities.characterOrCurrent(charIndex)
    local info = game.party.getCharacterInfo(charIndex, { "name", "class" })
    if info.class == game.ClassType.Lich then
        return info.name .. " is already a Lich!!", false
    end

    game.party.setCharacterInfo(charIndex, {
        class = game.ClassType.Lich
    })

    local lichJar = {
        id = game.ItemType.LichJarFull,
        holder = tonumber(charIndex)
    }
    game.party.addCustomItemToInventory(charIndex, lichJar)
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
