local logBindings = requireLogBindings()

---@type function[]
local callbacks = {}

local isLogging = false

--- The _globalLogSink is a global function called from c++ LogSink whenever a log message is created
---@param logLevel string
---@param message string
---@diagnostic disable-next-line: name-style-check
function _globalLogSink(logLevel, message)
    if isLogging then
        return
    end

    isLogging = true
    for _, callback in pairs(callbacks) do
        callback(logLevel, message)
    end
    isLogging = false
end

---@class LogListener
---@field register fun(callback: function)
local logListener = {
    ---Register a callback function accepting a log message ( look at the logsink function above )
    ---@param callback function
    register = function (callback)
        table.insert(callbacks, callback)
    end
}

return {
    bindings = logBindings,
    listener = logListener
}
