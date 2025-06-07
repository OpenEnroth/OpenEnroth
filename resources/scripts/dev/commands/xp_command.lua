local Utilities = require "dev.commands.command_utils"
local Game = require "bindings.game"

---Function that gives to the party a "xp" amount of Experience points and play the award animation on all characters
---@param xp integer - number of experience points to give to the party
---@return string    - the message sent back to the console
---@return boolean   - flag that tells if the command has been successful
local givePartyXp = function (xp)
    Game.party.givePartyXp(xp)
    Game.party.playAllCharactersAwardSound()
    return "Party gained " .. xp .. " xp", true
end

local subCommands = {
    {
        name = "get",
        callback = Utilities.showCharsProperty("xp"),
        description = "Shows the current experience points of all characters in the party."
    },
    {
        name = "rem",
        callback = Utilities.changeCharProperty("xp", Utilities.opType.rem, true, tonumber),
        params = {
            { name = "xp",   type = "number",         optional = false, description = "Amount of experience points to remove." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to remove experience points from." }
        },
        description = "Removes the specified amount of experience points from the character."
    },
    {
        name = "add",
        callback = Utilities.changeCharProperty("xp", Utilities.opType.add, true, tonumber),
        params = {
            { name = "xp",   type = "number",         optional = false, description = "Amount of experience points to add." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to add experience points to." }
        },
        description = "Adds the specified amount of experience points to the character."
    },
    {
        name = "set",
        callback = Utilities.changeCharProperty("xp", Utilities.opType.set, true, tonumber),
        params = {
            { name = "xp",   type = "number",         optional = false, description = "Amount of experience points to set." },
            { name = "char", type = "characterIndex", optional = false, description = "Character index to set experience points for." }
        },
        description = "Sets the character's experience points to the specified value."
    },
    {
        name = "party",
        callback = givePartyXp,
        params = {
            { name = "xp", type = "number", optional = false, description = "Amount of experience points to give to the party." }
        },
        description = "Gives the specified amount of experience points to the party."
    }
}

return {
    name = "xp",
    description = "Change the amount of experience points for the party.",
    details = "",
    subCommands = subCommands
}
