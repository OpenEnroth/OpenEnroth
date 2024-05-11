local Console = require "console"
local InputListener = require "core.input_listener"
local Input = require "bindings.input"
local Platform = require "bindings.platform"
local Overlay = require "bindings.overlay"
local Utilities = require "utils"
local imgui = Overlay.imgui

local isWindowMaximized = false
local baseColor = { 32, 32, 32, 255 }
local messageBkgColor = { 16, 16, 16, 255 }

local function getColorAlpha(col)
    local a = isWindowMaximized and 200 or 64
    return { col[1], col[2], col[3], a }
end

local function historyPrev()
    if Console.editTB.state[nk.EditState.NK_EDIT_ACTIVE] then
        Console:navigateHistory(-1)
        return true
    end
    return false
end

local function historyNext()
    if Console.editTB.state[nk.EditState.NK_EDIT_ACTIVE] then
        Console:navigateHistory(1)
        return true
    end
    return false
end

local scrollToBottom = false

local function drawCommandLineRow()
    if imgui.Button(Console.isExpanded and "<" or ">") then
        Console.isExpanded = not Console.isExpanded
    end
    imgui.SameLine()

    local text, changed = imgui.InputTextWithHint("##Input", "Write something here...", Console.text,
        imgui.ImGuiInputTextFlags.EnterReturnsTrue)
    Console.text = text

    if changed then
        if not Utilities.isEmpty(Console.text) then
            Console:send()
            scrollToBottom = true
            imgui.SetKeyboardFocusHere(-1)
        end
    end

    imgui.SameLine()
    if imgui.Button("Send") then
        if not Utilities.isEmpty(Console.text) then
            scrollToBottom = true
            Console:send()
        end
    end
end

local function drawOptionsRow()
    Console.logEnabled = imgui.Checkbox("Show Log", Console.logEnabled)
    imgui.SameLine()
    Console.autoMinimize = imgui.Checkbox("Auto Hide", Console.autoMinimize)
end

local function drawMessages()
    imgui.BeginChild("Messages", 0, -(imgui.GetFrameHeightWithSpacing() * 2), true)
    for i = 1, #Console.messages do
        local message = Console.messages[i]
        if message.source ~= "log" or Console.logEnabled then
            imgui.PushStyleColor(imgui.ImGuiCol.Text, message.col.r, message.col.g, message.col.b,
                message.col.a)
            imgui.TextUnformatted(message.text)
            imgui.PopStyleColor()
        end
    end
    if scrollToBottom then
        imgui.SetScrollHereY(1.0);
        scrollToBottom = false
    end
    imgui.EndChild()
end

local function drawConsole()
    local mainWinW, mainWinH = Platform.window.dimensions()
    Console:updateWindowSize(isWindowMaximized, mainWinW, mainWinH, 35)

    imgui.SetNextWindowPos(Console.rect.x, Console.rect.y)
    imgui.SetNextWindowSize(Console.rect.w, Console.rect.h)
    local _, drawWindow = imgui.Begin("Debug Console", true, imgui.ImGuiWindowFlags.NoTitleBar)
    if drawWindow then
        local flags = imgui.ImGuiHoveredFlags.RootAndChildWindows
        isWindowMaximized = imgui.IsWindowHovered(flags) or Console.autoMinimize
        if isWindowMaximized then
            drawMessages()
            drawOptionsRow()
        end
    end
    drawCommandLineRow()
    imgui.End()
end

local unregisterFromInput = function () end

local ConsoleOverlay = {}

ConsoleOverlay.init = function ()
    Console.scroll:set(0, 0)
    if #Console.messages == 0 then
        Console:addMessage("Type \"help\" on the command line to get a list of all the commands",
            Utilities.color(1, 1, 1, 0.5))
    end
    unregisterFromInput = InputListener.registerKeyPressBulk({
        { key = Input.PlatformKey.KEY_UP,   callback = historyPrev },
        { key = Input.PlatformKey.KEY_DOWN, callback = historyNext }
    })
end

ConsoleOverlay.update = function ()
    drawConsole()
end

ConsoleOverlay.close = function ()
    unregisterFromInput()
end

return ConsoleOverlay
