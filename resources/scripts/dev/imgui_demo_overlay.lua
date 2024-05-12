local imgui = require "bindings.overlay".imgui

local ImGuiDemoOverlay = {}

ImGuiDemoOverlay.init = function ()
end

ImGuiDemoOverlay.update = function ()
    imgui.ShowDemoWindow()
end

ImGuiDemoOverlay.close = function ()
end

return ImGuiDemoOverlay
