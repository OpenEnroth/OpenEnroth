local nk = require "bindings.overlay".nk
local window = require "bindings.platform".window
local Config = require "bindings.config"
local Console = require "console"

local CheatOverlay = {}

---@type table<string, string>
local availableCommands = {}

CheatOverlay.init = function ()
    local numberOfCommands = Config.getConfig("cheat_commands", "commands_number")
    for i = 1, numberOfCommands do
        local command = Config.getConfig("cheat_commands", string.format("command%02d", i))
        table.insert(availableCommands, command)
    end
end

CheatOverlay.close = function ()
end

CheatOverlay.update = function (ctx)
    local w, h = window.dimensions()
    nk.window_begin(ctx, "TEST", { x = w - 205, y = 5, w = 200, h = h - 10 }, { "scalable" })
    nk.layout_row_dynamic(ctx, 0, 1)
    for _, command in pairs(availableCommands) do
        if nk.button_label(ctx, command) then
            Console:send(command)
        end
    end
    nk.window_end(ctx)
end


return CheatOverlay
