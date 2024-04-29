--[[
 Initialize all the core scripts and bindings
]]
require "core.input"
require "core.error"
require "core.game"
require "core.logger"

--- @type OverlayBindings
local Overlay = require "bindings.overlay"
local ConsoleOverlay = require "dev.ui_debug_hud"

-- Register all the game console commands
local GameCommands = require "dev.commands.game_commands"
GameCommands.registerGameCommands()

Overlay.addOverlay("console", ConsoleOverlay)
