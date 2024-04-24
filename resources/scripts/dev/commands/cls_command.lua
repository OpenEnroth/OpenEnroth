local Console = require "console"

return {
    name = "cls",
    description = "clear console",
    callback = function ()
        Console:clear()
        return "", true
    end
}
