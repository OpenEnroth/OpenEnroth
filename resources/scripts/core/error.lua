local logger = requireLog()

---Global function called from c++ used to catch errors
---@param errorMessage string
---@diagnostic disable-next-line: name-style-check
_globalErrorHandler = function (errorMessage)
    logger.error(errorMessage)
end
