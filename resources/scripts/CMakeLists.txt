cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(SCRIPTS_SOURCES
    console.lua
    init.lua
    utils.lua
    core/error.lua
    core/input_listener.lua
    core/log_listener.lua
    dev/cheat_command_overlay.lua
    dev/console_overlay.lua
    dev/commands/alignment_command.lua
    dev/commands/class_command.lua
    dev/commands/cls_command.lua
    dev/commands/command_manager.lua
    dev/commands/command_utils.lua
    dev/commands/condition_command.lua
    dev/commands/config_command.lua
    dev/commands/debug_command.lua
    dev/commands/food_command.lua
    dev/commands/game_commands.lua
    dev/commands/gold_command.lua
    dev/commands/hp_command.lua
    dev/commands/inventory_command.lua
    dev/commands/mana_command.lua
    dev/commands/skillpoints_command.lua
    dev/commands/skills_command.lua
    dev/commands/xp_command.lua
    )

add_library(scripts INTERFACE ${SCRIPTS_SOURCES})
target_check_lua_style(scripts)