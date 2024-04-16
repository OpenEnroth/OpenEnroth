local command_manager = require "dev.commands.command_manager"
local config_commands = require "dev.commands.config_commands"
local gold_commands = require "dev.commands.gold_commands"
local xp_commands = require "dev.commands.xp_commands"
local sp_commands = require "dev.commands.skillpoints_commands"
local food_commands = require "dev.commands.food_commands"
local alignment_commands = require "dev.commands.alignment_commands"
local inventory_commands = require "dev.commands.inventory_commands"

local game_commands = {}

--- Register all the commands used in the dev console
game_commands.register_game_commands = function()
    command_manager.register("config", "Change any gameconfig value.", "Usage:\n\nconfig <action> [args]", config_commands)
    command_manager.register("gold", "Change the amount of gold.", "Usage:\n\ngold <action> [args]", gold_commands)
    command_manager.register("xp", "Change the amount of experience points for the party.", "Usage:\n\nxp <action> [args]", xp_commands)
    command_manager.register("sp", "Change the amount of skill points for the party.", "Usage:\n\nsp <action> [args]", sp_commands)
    command_manager.register("food", "Change the amount of food.", "Usage:\n\nfood <action> [args]", food_commands)
    command_manager.register("alignment", "Change the alignment of the party.", "Usage:\n\nalignment <action> [args]", alignment_commands)
    command_manager.register("inventory", "Add/Remove items from character inventory.", "", inventory_commands)
end

return game_commands