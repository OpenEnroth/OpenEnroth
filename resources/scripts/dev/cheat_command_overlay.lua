local nk = require "bindings.overlay".nk
local window = require "bindings.platform".window
local Config = require "bindings.config"
local Game = require "bindings.game"
local Console = require "console"
local Utilities = require "utils"

local CheatOverlay = {}

---@private
---@class CheatCommandEntry
---@field command string
---@field label string
---@field draw fun(entry: CheatCommandEntry, ctx:NuklearContext) : boolean

---@type table<string, CheatCommandEntry>
local availableCommands = {}

local enabledColor = { 50, 100, 50, 255 }
local disabledColor = { 100, 50, 50, 255 }
local enabledHoveredColor = { 25, 75, 25, 255 }
local disabledHoveredColor = { 75, 25, 25, 255 }

---@param ctx NuklearContext
---@param entry CheatCommandEntry
---@return boolean
local function defaultDrawCommandEntry(entry, ctx)
    return nk.button_label(ctx, entry.label)
end

-- The configValue is provided in the form of "[command to run] | (optional label)"
---@param configValue string
---@return CheatCommandEntry
local function createCheatCommandEntry(configValue)
    local pair = Utilities.splitString(configValue, "|")
    local command = pair[1];
    -- If the optional label is not provided we use the same command as button label
    local label = pair[2] and pair[2] or pair[1];

    local drawFunction = defaultDrawCommandEntry

    -- If the command to run is changing the value of a Boolean ConfigEntry we apply a special
    -- style based on the current boolean value
    local arguments = Utilities.splitString(command, " ")
    if arguments[1] == "config" and arguments[2] == "toggle" then
        local configName = arguments[3]
        drawFunction = function (entry, ctx)
            local valueString = Config.getConfig(configName)
            local color = Utilities.toBoolean(valueString) and enabledColor or disabledColor
            nk.style_push(ctx, "button", "normal", color)
            color = Utilities.toBoolean(valueString) and enabledHoveredColor or disabledHoveredColor
            nk.style_push(ctx, "button", "hover", color)
            local isPressed = nk.button_label(ctx, entry.label)
            nk.style_pop(ctx, "button", "normal")
            nk.style_pop(ctx, "button", "hover")
            return isPressed
        end
    else
    end

    return {
        command = command,
        label = label,
        draw = drawFunction
    }
end

CheatOverlay.init = function ()
    local numberOfCommands = Config.getConfig("cheat_commands", "commands_number")
    for i = 1, numberOfCommands do
        local command = Config.getConfig("cheat_commands", string.format("command%02d", i))
        table.insert(availableCommands, createCheatCommandEntry(command))
    end
end

CheatOverlay.close = function ()
end

CheatOverlay.update = function (ctx)
    local w, h = window.dimensions()
    nk.style_push(ctx, "window", "fixed_background", { 64, 64, 64, 128 })
    nk.window_begin(ctx, "Debug Menu", { x = w - 305, y = 5, w = 300, h = h - 10 }, { "title", "scalable", "movable" })
    nk.layout_row_dynamic(ctx, 0, 2)
    for _, entry in pairs(availableCommands) do
        if entry:draw(ctx) then
            --TODO(Gerark) I should expose the audio enum instead of using magic number
            -- It should look like this: Audio.playSound(SOUND_StartMainChoice02, SOUND_MODE_UI)
            Game.audio.playSound(75, 1)
            Console:send(entry.command)
        end
    end
    nk.window_end(ctx)
    nk.style_pop(ctx, "window", "fixed_background")
end


return CheatOverlay
