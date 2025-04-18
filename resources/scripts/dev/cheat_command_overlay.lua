local Overlay = require "bindings.overlay"
local imgui = Overlay.imgui
local window = require "bindings.platform".window
local Config = require "bindings.config"
local Audio = require "bindings.audio"
local Console = require "console"
local Utilities = require "utils"

local CheatOverlay = {}

---@private
---@class CheatCommandEntry
---@field command string
---@field label string
---@field draw fun(entry: CheatCommandEntry) : boolean

---@type table<string, CheatCommandEntry>
local availableCommands = {}

local enabledColor = Utilities.color(0.19, 0.39, 0.19, 1)
local disabledColor = Utilities.color(0.39, 0.19, 0.19, 1)
local enabledHoveredColor = Utilities.color(0.09, 0.29, 0.09, 1)
local disabledHoveredColor = Utilities.color(0.29, 0.09, 0.09, 1)
local buttonSize = { w = -1, h = 30 }

---@param entry CheatCommandEntry
---@return boolean
local function defaultDrawCommandEntry(entry)
    return imgui.button(entry.label, buttonSize.w, buttonSize.h)
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

    -- In case the command is changing a config we're going to display a visual indication
    -- based on the boolean value ( green or red background )
    local arguments = Utilities.splitString(command, " ")
    if arguments[1] == "config" and arguments[2] == "toggle" then
        local configName = arguments[3]
        drawFunction = function (entry)
            local valueString = Config.entry(configName).value
            local color = Utilities.toBoolean(valueString) and enabledColor or disabledColor
            imgui.pushStyleColor(imgui.ImGuiCol.ButtonHovered, color.r, color.g, color.b, color.a)
            color = Utilities.toBoolean(valueString) and enabledHoveredColor or disabledHoveredColor
            imgui.pushStyleColor(imgui.ImGuiCol.Button, color.r, color.g, color.b, color.a)
            local isPressed = imgui.button(entry.label, buttonSize.w, buttonSize.h)
            imgui.popStyleColor()
            imgui.popStyleColor()
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
    local numberOfCommands = 40
    for i = 1, numberOfCommands do
        local command = Config.entry("cheat_commands", string.format("command%02d", i)).value
        if command and not Utilities.isEmpty(command) then
            table.insert(availableCommands, createCheatCommandEntry(command))
        end
    end
end

CheatOverlay.close = function ()
end

local function drawColumnItem(entry, columnIndex, itemsPerRow)
    if columnIndex == 0 then
        imgui.tableNextRow();
    end
    imgui.tableSetColumnIndex(columnIndex);
    if entry:draw() then
        --TODO(Gerark) I should expose the audio enum instead of using magic number
        -- It should look like this: Audio.playSound(SOUND_StartMainChoice02, SOUND_MODE_UI)
        Audio.playSound(75, 1)
        Console:send(entry.command)
    end
    return columnIndex + 1 < itemsPerRow and columnIndex + 1 or 0
end

local function beginTable(name, columnSize)
    local w, _ = imgui.getWindowSize()
    imgui.beginTable(name, math.max(1, math.floor(w / columnSize)))
end

CheatOverlay.update = function ()
    local screenW, screenH = window.dimensions()
    imgui.setNextWindowSize(300, screenH - 10, imgui.ImGuiCond.FirstUseEver);
    imgui.setNextWindowPos(screenW - 305, 5, imgui.ImGuiCond.FirstUseEver);
    if imgui.beginWindow("Debug Menu") then
        beginTable("buttonTable", 150)
        local columnIndex = 0
        for _, entry in pairs(availableCommands) do
            columnIndex = drawColumnItem(entry, columnIndex, imgui.tableGetColumnCount())
        end
        imgui.endTable()
    end
    imgui.endWindow()
end


return CheatOverlay
