local GameCommands = require "dev.commands.game_commands"

-- TODO(Gerark) - Fix ui_init entry point in c++
---@diagnostic disable-next-line: name-style-check, lowercase-global
function ui_init()
    GameCommands.register_game_commands()
end
