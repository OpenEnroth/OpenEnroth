local Game = require "bindings.game"

local subCommands = {
    {
        name = "trigger",
        callback = Game.debugCallback,
        description = "Run debug callback."
    },
}

return {
    name = "debug",
    description = "Dev commands to run throwaway C++ code.",
    details = "",
    subCommands = subCommands
}
