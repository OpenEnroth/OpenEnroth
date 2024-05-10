local CommandManager = require "dev.commands.command_manager"
local ConfigCommand = require "dev.commands.config_command"
local GoldCommand = require "dev.commands.gold_command"
local XpCommand = require "dev.commands.xp_command"
local SpCommand = require "dev.commands.skillpoints_command"
local FoodCommand = require "dev.commands.food_command"
local AlignmentCommand = require "dev.commands.alignment_command"
local InventoryCommand = require "dev.commands.inventory_command"
local LuaCommand = require "dev.commands.run_lua_command"
local ClearConsoleCommand = require "dev.commands.cls_command"
local ConditionCommand = require "dev.commands.condition_command"
local HpCommand = require "dev.commands.hp_command"
local ManaCommand = require "dev.commands.mana_command"
local SkillsCommand = require "dev.commands.skills_command"
local ClassCommand = require "dev.commands.class_command"

local Renderer = require "bindings.renderer"

local reloadShadersCommand = {
    name = "reload_shaders",
    description = "Reload all shaders",
    callback = function ()
        Renderer.reloadShaders()
        return "", true
    end
}

-- todo(Gerark) work in progress, need to expose in c++ the functionalities to reload the scripts
--local reloadScriptsCommand = {
--    name = "reload_scripts",
--    description = "Reload all scripts",
--    callback = function()
--        return "Not implemented yet", false
--    end
--}

--- @class GameCommands
local GameCommands = {}

--- Register all the commands used in the dev console
GameCommands.registerGameCommands = function ()
    CommandManager.register(ConfigCommand)
    CommandManager.register(GoldCommand)
    CommandManager.register(XpCommand)
    CommandManager.register(SpCommand)
    CommandManager.register(FoodCommand)
    CommandManager.register(AlignmentCommand)
    CommandManager.register(InventoryCommand)
    CommandManager.register(LuaCommand)
    CommandManager.register(ClearConsoleCommand)
    CommandManager.register(reloadShadersCommand)
    CommandManager.register(ConditionCommand)
    CommandManager.register(HpCommand)
    CommandManager.register(ManaCommand)
    CommandManager.register(SkillsCommand)
    CommandManager.register(ClassCommand)
end

return GameCommands
