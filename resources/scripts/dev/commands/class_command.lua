local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

local function become_lich(char_index)
    char_index = utils.character_or_current(char_index)
    local info = mm.party.get_character_info(char_index, { "name", "class" })
    if info.class == mm.ClassType.Lich then
        return info.name.." is already a Lich!!", false
    end

    mm.party.set_character_info(char_index, {
        class = mm.ClassType.Lich
    })

    local lich_jar = {
        id = mm.ItemType.LichJarFull,
        holder = tonumber(char_index)
    }
    mm.party.add_custom_item_to_inventory(char_index, lich_jar)
    return info.name.." became a Lich.", true
end

local subcommands = {
    lich = become_lich
}

return {
    name = "class",
    description = "Change and get information about character class",
    callback = subcommands
}

} } -- ADDING RANDOM BRACES TO FORCE AN ERROR FOR LuaLS