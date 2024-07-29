require "core.error"

local Overlay = require "bindings.overlay"
local ConsoleOverlay = require "dev.console_overlay"
--local ImGuiDemo = require "dev.imgui_demo_overlay"
local CheatOverlay = require "dev.cheat_command_overlay"
local GameCommands = require "dev.commands.game_commands"

GameCommands.registerGameCommands()

Overlay.addOverlay("console", ConsoleOverlay)
Overlay.addOverlay("cheatTable", CheatOverlay)
--Overlay.addOverlay("demo", ImGuiDemo)
