local Console = require "console"
local Platform = require "bindings.platform"
local Overlay = require "bindings.overlay"
local Utilities = require "utils"
local imgui = Overlay.imgui

local scrollToBottom = false
local consoleMargin = 5
local footerHeight = 35
local minWidth = 200
local isWindowMaximized = false

---@param w integer
---@param h integer
---@return table
local function calculateWindowSize(w, h)
    local consoleWidth = w / 3
    ---@type Rect
    if isWindowMaximized then
        return Utilities.rect(
            consoleMargin,
            consoleMargin,
            consoleWidth,
            h - consoleMargin * 2
        )
    else
        return Utilities.rect(
            consoleMargin,
            h - footerHeight - consoleMargin,
            consoleWidth,
            footerHeight
        )
    end
end

local function inputTextCallback(callbackEvent, param)
    if callbackEvent == imgui.ImGuiInputTextFlags.CallbackHistory then
        Console:navigateHistory(param)
        return Console.text
    end
end

local function drawCommandLineRow()
    --- @type any
    local inputTextFlags = bit32.bor(imgui.ImGuiInputTextFlags.EnterReturnsTrue,
        imgui.ImGuiInputTextFlags.CallbackHistory)
    local text, changed = imgui.InputTextWithHint("##Input", "Write something here...", Console.text, inputTextFlags,
        inputTextCallback)
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

    imgui.SameLine()
    if imgui.Button("Clear") then
        Console:clear()
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
            imgui.TextWrapped(message.text)
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
    local rect = calculateWindowSize(mainWinW, mainWinH)

    imgui.SetNextWindowPos(rect.x, rect.y)
    imgui.SetNextWindowSize(rect.w, rect.h, imgui.ImGuiCond.FirstUseEver)
    imgui.SetNextWindowSizeConstraints(minWidth, rect.h, mainWinW - consoleMargin * 2, rect.h)
    local _, drawWindow = imgui.Begin("Debug Console", true, imgui.ImGuiWindowFlags.NoTitleBar)
    if drawWindow then
        local flags = imgui.ImGuiHoveredFlags.RootAndChildWindows
        isWindowMaximized = imgui.IsWindowHovered(flags) or imgui.IsMouseHoveringRect(rect.x, rect.y, rect.w, rect.h) or
            not Console.autoMinimize
        if isWindowMaximized then
            drawMessages()
            drawOptionsRow()
        end
    end
    drawCommandLineRow()
    imgui.End()
end

local ConsoleOverlay = {}

ConsoleOverlay.init = function ()
    if #Console.messages == 0 then
        Console:addMessage("Type \"help\" on the command line to get a list of all the commands",
            Utilities.color(1, 1, 1, 0.5))
    end
end

ConsoleOverlay.update = function ()
    drawConsole()
end

ConsoleOverlay.close = function ()
end

return ConsoleOverlay
