local game_commands = require "dev.commands.game_commands"

function ui_init()
    game_commands.register_game_commands()
end