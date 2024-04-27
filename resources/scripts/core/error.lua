local logger = require "core.logger".bindings

---Global function called from c++ used to catch errors
---@param errorMessage string
---@diagnostic disable-next-line: name-style-check
_globalErrorHandler = function (errorMessage)
    logger.error(errorMessage)
end
