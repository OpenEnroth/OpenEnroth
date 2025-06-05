local CommandManager = require "dev.commands.command_manager"
local CommandUtilities = require "dev.commands.command_utils"
local Console = require "console"
local Game = require "bindings.game"
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
    local text, changed = imgui.inputTextWithHint("##Input", "Write something here...", Console.text, inputTextFlags,
        inputTextCallback)
    Console.text = text

    if changed then
        if not Utilities.isEmpty(Console.text) then
            Console:send()
            scrollToBottom = true
            imgui.setKeyboardFocusHere(-1)
        end
    end

    imgui.sameLine()
    if imgui.button("Send") then
        if not Utilities.isEmpty(Console.text) then
            scrollToBottom = true
            Console:send()
        end
    end

    imgui.sameLine()
    if imgui.button("Clear") then
        Console:clear()
    end
end

local function drawOptionsRow()
    Console.logEnabled = imgui.checkbox("Show Log", Console.logEnabled)
    imgui.sameLine()
    Console.autoMinimize = imgui.checkbox("Auto Hide", Console.autoMinimize)
end

local function drawMessages()
    imgui.beginChild("Messages", 0, -(imgui.getFrameHeightWithSpacing() * 3), true, imgui.ImGuiWindowFlags.None)
    for i = 1, #Console.messages do
        local message = Console.messages[i]
        if message.source ~= "log" or Console.logEnabled then
            imgui.pushStyleColor(imgui.ImGuiCol.Text, message.col.r, message.col.g, message.col.b,
                message.col.a)
            imgui.textWrapped(message.text)
            imgui.popStyleColor()
        end
    end
    if scrollToBottom then
        imgui.setScrollHereY(1.0)
        scrollToBottom = false
    end
    imgui.endChild()
end

--- Render a leaf menu command with parameters
--- @param command Command
--- @param commandPath string
--- @param tokens table<integer, string>
--- @return boolean
local function renderLeafMenuCommand(command, commandPath, tokens)
    local isSelected = false
    if imgui.beginMenu(command.name) then
        local canExecute = true

        imgui.pushStyleColor(imgui.ImGuiCol.Text, 0.25, 0.57, 0.87, 1)
        imgui.textUnformatted(command.description or "No description available.")
        imgui.popStyleColor()
        imgui.separator()

        local hasParams = command.params ~= nil
        if hasParams then
            if Console.dataParameters.path ~= commandPath then
                Console.dataParameters.path = commandPath
                Console.dataParameters.params = {}
                for _, param in ipairs(command.params) do
                    if param.type == "characterIndex" then
                        Console.dataParameters.params[param.name] = { value = tostring(Game.party.getActiveCharacter()) }
                    elseif param.type == "enum" then
                        Console.dataParameters.params[param.name] = {
                            value = param.defaultValue and
                                param.defaultValue() or ""
                        }
                    else
                        Console.dataParameters.params[param.name] = { value = "" }
                    end
                end
            end
            for _, paramInfo in ipairs(command.params) do
                local label = paramInfo.name .. (paramInfo.optional and "(optional)" or "")
                imgui.alignTextToFramePadding()
                imgui.textUnformatted(label .. ": ")

                local dataParam = Console.dataParameters.params[paramInfo.name]

                local hasChanged = false
                if paramInfo.renderer ~= nil then
                    hasChanged = paramInfo.renderer(dataParam, paramInfo, Console.dataParameters.params)
                else
                    hasChanged = CommandUtilities.defaultParamRenderer(paramInfo.type, dataParam, paramInfo,
                        Console.dataParameters.params)
                end

                if hasChanged then
                    for _, otherParamInfo in ipairs(command.params) do
                        if otherParamInfo.resetOnOtherParamChange and otherParamInfo ~= paramInfo then
                            local otherDataParam = Console.dataParameters.params[otherParamInfo.name]
                            otherDataParam.value = otherParamInfo.resetOnOtherParamChange()
                        end
                    end
                end

                if not paramInfo.optional and Utilities.isEmpty(dataParam.value) then
                    canExecute = false
                    imgui.sameLine()
                    imgui.pushStyleColor(imgui.ImGuiCol.Text, 1, 0, 0, 1) -- Red color for required params
                    imgui.textUnformatted("(required)")
                    imgui.popStyleColor()
                elseif not Utilities.canBeConvertedTo(dataParam.value, paramInfo.type) then
                    canExecute = false
                    imgui.sameLine()
                    imgui.pushStyleColor(imgui.ImGuiCol.Text, 1, 0.5, 0, 1) -- Orange color for invalid params
                    imgui.textUnformatted("(invalid - expected " .. paramInfo.type .. ")")
                    imgui.popStyleColor()
                end
            end
            imgui.separator()
        end

        imgui.dummy(200, 0)
        if imgui.menuItem("Execute", canExecute) then
            isSelected = true
            if hasParams then
                for _, paramInfo in ipairs(command.params) do
                    local dataParam = Console.dataParameters.params[paramInfo.name]
                    if not paramInfo.optional or (paramInfo.optional and not Utilities.isEmpty(dataParam.value)) then
                        table.insert(tokens, dataParam.value)
                    end
                end
            end
        end

        imgui.endMenu()
    end
    return isSelected
end

--- Recursively render menu commands and their subcommands
--- @param command Command
--- @param commandPath string
--- @param tokens table<integer, string>
--- @return boolean
local function recursiveMenuCommand(command, commandPath, tokens)
    local isSelected = false

    if command.subCommands ~= nil then
        if imgui.beginMenu(command.name) then
            for _, subCommand in ipairs(command.subCommands) do
                if subCommand.name ~= "default" then
                    isSelected = recursiveMenuCommand(subCommand, commandPath .. subCommand.name, tokens)
                    if isSelected then
                        table.insert(tokens, 1, subCommand.name)
                        break
                    end
                end
            end

            imgui.endMenu()
        end
    else
        isSelected = renderLeafMenuCommand(command, commandPath, tokens)
    end

    return isSelected
end

local function drawCommandsMenuBar()
    imgui.beginChild("menuBar", 0, imgui.getFrameHeightWithSpacing() - 5, false, imgui.ImGuiWindowFlags.MenuBar)
    imgui.beginMenuBar()
    Console.isMenuBarShown = imgui.beginMenu("Commands")
    if Console.isMenuBarShown then
        --- @type table<integer, Command>
        local sortedCommands = {}
        for _, command in pairs(CommandManager.list) do
            table.insert(sortedCommands, command)
        end
        table.sort(sortedCommands, function (a, b)
            return a.name < b.name
        end)

        for _, command in ipairs(sortedCommands) do
            --- @type table<integer, string>
            local tokens = {}
            if recursiveMenuCommand(command, command.name, tokens) then
                Console.text = command.name
                for _, token in ipairs(tokens) do
                    Console.text = Console.text .. " " .. token
                end
                Console:send()
                Console.dataParameters.params = {}
                Console.dataParameters.path = ""
            end
        end
        imgui.endMenu()
    end
    imgui.endMenuBar()
    imgui.endChild()
end

local function drawConsole()
    local mainWinW, mainWinH = Platform.window.dimensions()
    local rect = calculateWindowSize(mainWinW, mainWinH)

    imgui.setNextWindowPos(rect.x, rect.y)
    imgui.setNextWindowSize(rect.w, rect.h, imgui.ImGuiCond.FirstUseEver)
    imgui.setNextWindowSizeConstraints(minWidth, rect.h, mainWinW - consoleMargin * 2, rect.h)

    local shouldDraw, _ = imgui.beginWindow("Debug Console", true, imgui.ImGuiWindowFlags.NoTitleBar)
    if shouldDraw then
        local flags = imgui.ImGuiHoveredFlags.RootAndChildWindows
        isWindowMaximized = imgui.isWindowHovered(flags) or imgui.isMouseHoveringRect(rect.x, rect.y, rect.w, rect.h) or
            not Console.autoMinimize or Console.isMenuBarShown
        if isWindowMaximized then
            drawMessages()
            drawOptionsRow()
            drawCommandsMenuBar()
        end
    end

    drawCommandLineRow()
    imgui.endWindow()
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
