local Utilities = require "dev.commands.command_utils"
local MM = require "mmbindings"

---Function that gives to the party a "xp" amount of Experience points and play the award animation on all characters
---@param xp integer - number of experience points to give to the party
---@return string    - the message sent back to the console
---@return boolean   - flag that tells if the command has been successful
local givePartyXp = function (xp)
    MM.party.give_party_xp(xp)
    MM.party.play_all_characters_award_sound()
    return "Party gained " .. xp .. " xp", true
end

local subCommands = {
    get = Utilities.showCharsProperty("xp"),
    rem = Utilities.changeCharProperty("xp", Utilities.opType.rem, true, tonumber),
    add = Utilities.changeCharProperty("xp", Utilities.opType.add, true, tonumber),
    set = Utilities.changeCharProperty("xp", Utilities.opType.set, true, tonumber),
    party = givePartyXp,
    default = Utilities.showCharsProperty("xp")
}

return {
    name = "xp",
    description = "Change the amount of experience points for the party.",
    details = "",
    callback = subCommands
}
