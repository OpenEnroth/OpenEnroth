local command_manager = require "dev.commands.command_manager"
local config_command = require "dev.commands.config_command"
local gold_command = require "dev.commands.gold_command"
local xp_command = require "dev.commands.xp_command"
local sp_command = require "dev.commands.skillpoints_command"
local food_command = require "dev.commands.food_command"
local alignment_command = require "dev.commands.alignment_command"
local inventory_command = require "dev.commands.inventory_command"
local lua_command = require "dev.commands.run_lua_command"
local clear_console_command = require "dev.commands.cls_command"
local condition_command = require "dev.commands.condition_command"
local hp_command = require "dev.commands.hp_command"
local mana_command = require "dev.commands.mana_command"
local skills_command = require "dev.commands.skills_command"
local class_command = require "dev.commands.class_command"

local mm = require "mmbindings"

local reload_shaders_command = {
    name = "reload_shaders",
    description = "Reload all shaders",
    callback = function()
        mm.render.reload_shaders()
        return "", true
    end
}

-- todo(Gerark) work in progress, need to expose in c++ the functionalities to reload the scripts
local reload_scripts_command = {
    name = "reload_shaders",
    description = "Reload all scripts",
    callback = function()
        return "Not implemented yet", false
    end
}

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
    command_manager.register(clear_console_command)
    command_manager.register(reload_shaders_command)
    command_manager.register(condition_command)
    command_manager.register(hp_command)
    command_manager.register(mana_command)
    command_manager.register(skills_command)
    command_manager.register(class_command)
end

return game_commands
