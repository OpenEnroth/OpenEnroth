local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

local function becomeLich(charIndex)
    charIndex = Utilities.characterOrCurrent(charIndex)
    local info = MM.party.get_character_info(charIndex, { "name", "class" })
    if info.class == MM.ClassType.Lich then
        return info.name .. " is already a Lich!!", false
    end

    MM.party.set_character_info(charIndex, {
        class = MM.ClassType.Lich
    })

    local lichJar = {
        id = MM.ItemType.LichJarFull,
        holder = tonumber(charIndex)
    }
    MM.party.add_custom_item_to_inventory(charIndex, lichJar)
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

} } -- ADDING RANDOM BRACES TO FORCE AN ERROR FOR LuaLS