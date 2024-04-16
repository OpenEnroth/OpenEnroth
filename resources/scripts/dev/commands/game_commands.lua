local command_manager = require "dev.commands.command_manager"
local config_command = require "dev.commands.config_command"
local gold_command = require "dev.commands.gold_command"
local xp_command = require "dev.commands.xp_command"
local sp_command = require "dev.commands.skillpoints_command"
local food_command = require "dev.commands.food_command"
local alignment_command = require "dev.commands.alignment_command"
local inventory_command = require "dev.commands.inventory_command"
local lua_command = require "dev.commands.run_lua_command"

local game_commands = {}

--- Register all the commands used in the dev console
game_commands.register_game_commands = function()
    command_manager.register(config_command)
    command_manager.register(gold_command)
    command_manager.register(xp_command)
    command_manager.register(sp_command)
    command_manager.register(food_command)
    command_manager.register(alignment_command)
    command_manager.register(inventory_command)
    command_manager.register(lua_command)
end

return game_commands
