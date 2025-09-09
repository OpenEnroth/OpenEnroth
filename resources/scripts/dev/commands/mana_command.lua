local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

local function showCharsMana()
    local count = Game.party.getPartySize()
    local message = ""
    for i = 1, count do
        local info = Game.party.getCharacterInfo(i, { "name", "mana", "maxMana" })
        message = message .. info.name .. ": " .. info.mana .. "/" .. info.maxMana .. "\n"
    end
    return message, true
end

local subCommands = {
    {
        name = "get",
        callback = showCharsMana,
        description = "Shows the current mana of all characters in the party."
    },
    {
        name = "add",
        callback = Utilities.changeCharProperty("mana", Utilities.opType.add, true, tonumber),
        params = {
            { name = "mana", type = "number",         optional = false, description = "Amount of mana to add." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to add mana to." }
        },
        description = "Adds the specified amount of mana to the character."
    },
    {
        name = "rem",
        callback = Utilities.changeCharProperty("mana", Utilities.opType.rem, true, tonumber),
        params = {
            { name = "mana", type = "number",         optional = false, description = "Amount of mana to remove." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to remove mana from." }
        },
        description = "Removes the specified amount of mana from the character."
    },
    {
        name = "set",
        callback = Utilities.changeCharProperty("mana", Utilities.opType.set, true, tonumber),
        params = {
            { name = "mana", type = "number",         optional = false, description = "Amount of mana to set." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to set mana to." }
        },
        description = "Sets the character's mana to the specified value."
    }
}

return {
    name = "mana",
    description = "Show and manipulate characters mana.",
    details = "",
    subCommands = subCommands
}
