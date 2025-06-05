local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

local function becomeLich(charIndex)
    charIndex = Utilities.characterOrCurrent(charIndex)
    local info = Game.party.getCharacterInfo(charIndex, { "name", "class" })
    if info.class == Game.ClassType.Lich then
        return info.name .. " is already a Lich!!", false
    end

    Game.party.setCharacterInfo(charIndex, {
        class = Game.ClassType.Lich
    })

    local lichJar = {
        id = Game.ItemType.LichJarFull,
        holder = tonumber(charIndex)
    }
    Game.party.addCustomItemToInventory(charIndex, lichJar)
    return info.name .. " became a Lich.", true
end

local subCommands = {
    {
        name = "lich",
        callback = becomeLich,
        params = {
            { name = "char", type = "characterIndex", optional = true, description = "Character index to become a Lich. Defaults to current character." }
        },
        description = "Changes the character's class to Lich and gives them a Lich Jar."
    }
}

return {
    name = "class",
    description = "Change and get information about character class",
    subCommands = subCommands
}
