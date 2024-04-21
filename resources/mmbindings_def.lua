-- @meta

-- A meta file is here only to extract types and definitions. Any value assigned has no meaning except for helping with the definition
-- TODO(Gerark) most of these definitions must be changed on the c++ to follow the correct naming convention

---@type integer
---@diagnostic disable-next-line: name-style-check
NK_EDIT_COMMITED = 0
---@type integer
---@diagnostic disable-next-line: name-style-check
NK_EDIT_INACTIVE = 1
---@type integer
---@diagnostic disable-next-line: name-style-check
NK_EDIT_ACTIVE = 2

---@class nk_scroll
---@field new fun(x: integer, y: integer): table
---@diagnostic disable-next-line: name-style-check
nk_scroll = {}

--- Initialize the table with all the game c++ bindings
---@return table
---@diagnostic disable-next-line: name-style-check
function initMMBindings()
    return {}
end
