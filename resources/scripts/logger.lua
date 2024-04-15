--[[
 Example:
 local logger = require "logger"
 logger.register(function(level, message) {
    print("I'M CALLED WHEN A LOG MESSAGE IS CREATED")
 })
]]

---@type function[]
local callbacks = {}

--- The logsink is a global function called from c++ LogSink whenever a log message is created
--- If you change the variable name be sure to do the same in c++!!!
---@param log_level string
---@param message string
logsink = function(log_level, message)
    for key, callback in pairs(callbacks) do
        callback(log_level, message)
    end
end

return {
    ---Register a callback function accepting a log message ( look at the logsink function above )
    ---@param callback function
    register = function(callback)
        table.insert(callbacks, callback)
    end
}
