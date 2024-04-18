local utils = require "dev.commands.command_utils"
local mm = require "mmbindings"

---Function that gives to the party a "xp" amount of Experience points and play the award animation on all characters
---@param xp integer - number of experience points to give to the party
---@return string    - the message sent back to the console
---@return boolean   - flag that tells if the command has been successful
local give_party_xp = function(xp)
    mm.game.give_party_xp(xp)
    mm.game.play_all_characters_award_sound()
    return "Party gained "..xp.." xp", true
end

local subcommands = {
    get = utils.show_chars_property("xp"),
    rem = utils.change_char_property("xp", utils.OP_TYPE.rem, true, tonumber),
    add = utils.change_char_property("xp", utils.OP_TYPE.add, true, tonumber),
    set = utils.change_char_property("xp", utils.OP_TYPE.set, true, tonumber),
    party = give_party_xp,
    default = utils.show_chars_property("xp")
}

return {
    name = "xp",
    description = "Change the amount of experience points for the party.",
    details = "",
    callback = subcommands
}
