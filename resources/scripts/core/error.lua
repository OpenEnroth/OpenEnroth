local Log = require "bindings.log"

---Global function called from c++ used to catch errors
---@param errorMessage string
---@diagnostic disable-next-line: name-style-check
_globalErrorHandler = function (errorMessage)
    Log.error(debug.traceback(errorMessage))
end
