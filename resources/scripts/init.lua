--[[
 Initialize all the core scripts and bindings
]]
require "core.input"
require "core.error"
require "core.game"
require "core.logger"

-- Register all the game console commands
local GameCommands = require "dev.commands.game_commands"
GameCommands.registerGameCommands()
