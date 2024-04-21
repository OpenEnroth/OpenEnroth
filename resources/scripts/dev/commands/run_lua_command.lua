-- The scoped_environment is created to avoid conflicts with other global variables defined by the game
-- By doing so we won't mess with some important variables and we can play with lua commands in a safe way
local scopedEnvironment = {}
setmetatable(scopedEnvironment, { __index = _G })

local function runLua(code)
    if code and type(code) == "string" then
        local codeToRun, error = load(code, nil, nil, scopedEnvironment)
        if codeToRun then
            ---@type boolean, any
            local _, result = pcall(codeToRun)
            return result and result or "", true
        else
            return "Invalid code: " .. error, true
        end
    else
        return "Invalid lua code provided", false
    end
end

return {
    name = "lua",
    description = "Run custom lua code.",
    details = "",
    callback = runLua,
    customParser = function (paramsString) return { paramsString } end
}
