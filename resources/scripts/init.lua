--[[
 Initialize all the core scripts and bindings
]]
require "core.input"
require "core.error"
require "core.game"
require "core.logger"
require "core.nuklear"
require "core.debugView"

--- @type DebugViewBindings
local DebugView = require "bindings.debugView"
local ConsoleView = require "dev.ui_debug_hud"

-- Register all the game console commands
local GameCommands = require "dev.commands.game_commands"
GameCommands.registerGameCommands()

DebugView.addView(ConsoleView)
