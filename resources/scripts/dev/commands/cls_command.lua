local console = require "console"

return {
    name = "cls",
    description = "clear console",
    callback = function()
        console:clear()
        return "", true
    end
}
