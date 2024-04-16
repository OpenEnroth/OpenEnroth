-- The scoped_environment is created to avoid conflicts with other global variables defined by the game
-- By doing so we won't mess with some important variables and we can play with lua commands in a safe way
local scoped_environment = {}
setmetatable(scoped_environment, {__index = _G})

local function run_lua(code)
    if code and type(code) == "string" then
        local codeToRun, error = load(code, nil, nil, scoped_environment)
        if codeToRun then
            local result = codeToRun()
            return result and codeToRun(result) or "", true
        else
            return "Invalid code: "..error, true
        end
    else
        return "Invalid lua code provided", false
    end
end

return {
    name = "lua",
    description = "Run custom lua code.",
    details = "",
    callback = run_lua,
    custom_parser = function(params_string) return { params_string } end
}