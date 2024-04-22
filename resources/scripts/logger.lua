--[[
 Example:
 local logger = require "logger"
 logger.register(function(level, message) {
    print("I'M CALLED WHEN A LOG MESSAGE IS CREATED")
 })
]]

---@type function[]
local callbacks = {}

local isLogging = false

--- The logSink is a global function called from c++ LogSink whenever a log message is created
--- If you change the variable name be sure to do the same in c++!!!
---@param logLevel string
---@param message string
function logSink(logLevel, message)
    if isLogging then
        return
    end

    isLogging = true
    for _, callback in pairs(callbacks) do
        callback(logLevel, message)
    end
    isLogging = false
end

return {
    ---Register a callback function accepting a log message ( look at the logsink function above )
    ---@param callback function
    register = function (callback)
        table.insert(callbacks, callback)
    end
}
