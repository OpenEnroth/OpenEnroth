---@class Utilities
local Utilities = {}

---Convert a string to a boolean
---@param value string
---@return boolean
Utilities.toBoolean = function (value)
    return value ~= "false" and value ~= "0"
end

---Convert a variable to a number. in case of failure the defaultValue is returned
---@param amount any
---@param defaultValue number
---@return number
Utilities.toNumberOr = function (amount, defaultValue)
    amount = tonumber(amount)
    return amount and amount or defaultValue
end

--- Tell if a string is empty or nil
---@param s string
---@return boolean
Utilities.isEmpty = function (s)
    return s == nil or s == ""
end

--- Find an element inside the table using a function
---@param t table<any, any> - table to be searched
---@param func fun(value:any):boolean - function that returns true if the value matches the search criteria
---@return any - the key and value of the first element that matches the criteria, or nil if not found
Utilities.findIf = function (t, func)
    for _, v in ipairs(t) do
        if func(v) then
            return v
        end
    end
    return nil
end

--- Find the index of an element inside the table using a function
--- @param t table<any, any> - table to be searched
--- @param func fun(value:any):boolean - function that returns true if the value matches the search criteria
--- @return integer|nil - the index of the first element that matches the criteria, or nil if not found
Utilities.findIndex = function (t, func)
    for i, v in ipairs(t) do
        if func(v) then
            return i
        end
    end
    return nil
end

--- Split the string according to the separator, respecting quoted substrings
---@param str string - base string
---@param separator string - separator to be used
---@return table<integer, string> - list of splitted strings
Utilities.splitString = function (str, separator)
    separator = separator or "%s"
    local result = {}
    local len = #str
    local i = 1
    local current = ""
    local inQuotes = false
    local quoteChar = nil
    local hadQuotes = false

    while i <= len do
        local c = str:sub(i, i)

        if inQuotes then
            if c == "\\" and i < len then
                current = current .. str:sub(i + 1, i + 1)
                i = i + 2
            elseif c == quoteChar then
                inQuotes = false
                hadQuotes = true
                i = i + 1
            else
                current = current .. c
                i = i + 1
            end
        else
            if c == '"' or c == "'" then
                inQuotes = true
                quoteChar = c
                hadQuotes = false
                i = i + 1
            elseif c:match(separator) then
                if #current > 0 or hadQuotes then
                    table.insert(result, current)
                    current = ""
                    hadQuotes = false
                end
                i = i + 1
                while i <= len and str:sub(i, i):match(separator) do
                    i = i + 1
                end
            else
                current = current .. c
                i = i + 1
            end
        end
    end

    if #current > 0 or hadQuotes then
        table.insert(result, current)
    end

    return result
end

--- Take an enum table and returns the string representation of the value provided
--- It basically returns the key of the corresponding row
---
--- Example:
--- Game.SkillType = {
---    Club = 0,
---    Sword = 1,
---    Dagger = 2,
--- }
--- local result = enumToString(Game.SkillType, 2)
--- The value of variable "result" is "Dagger"
---
---@param enumTable any - The enum table ( ex: Game.SkillType, Game.SkillMastery )
---@param value any
---@return string|nil
function enumToString(enumTable, value)
    ---@cast enumTable table<string, any>
    for k, v in pairs(enumTable) do
        if v == value then
            return k
        end
    end

    return nil
end

---@param enumTable any - The enum table ( ex: Game.SkillType, Game.SkillMastery )
---@param valueStr string
---@return any
function stringToEnum(enumTable, valueStr)
    ---@cast enumTable table<string, any>
    for k, v in pairs(enumTable) do
        if k == valueStr then
            return v
        end
    end

    return nil
end

--- Convert an enum table to string containing all the keys separated by \0 (zero) separator. Useful for imgui combo boxes
--- @param enumTable any - The enum table ( ex: Game.SkillType, Game.SkillMastery )
--- @return string - A zero-separated list of strings representing the enum keys
function enumTableToZeroSeparatedList(enumTable)
    ---@cast enumTable table<string, any>
    local result = {}
    for k, _ in pairs(enumTable) do
        table.insert(result, k)
    end
    return table.concat(result, "\0") .. "\0"
end

--- Get the nth element of a table treated as a map
--- @param t table<any, any> - The table to be searched
--- @param index integer - The index of the element to be retrieved
--- @return any - The value of the nth element, or nil if the index is out of bounds
Utilities.getValueAtIndex = function (t, index)
    local i = 0
    for _, v in pairs(t) do
        i = i + 1
        if i == index then
            return v
        end
    end
    return nil
end

--- Get the nth key of a table treated as a map
--- @param t table<any, any> - The table to be searched
--- @param index integer - The index of the key to be retrieved
--- @return any - The key of the nth element, or nil if the index is out of bounds
Utilities.getKeyAtIndex = function (t, index)
    local i = 0
    for k, _ in pairs(t) do
        i = i + 1
        if i == index then
            return k
        end
    end
    return nil
end

--- Get the index of a value in a table treated as a map
--- @param t table<any, any> - The table to be searched
--- @param value any - The value to be searched
--- @return integer|nil - The zero-based index of the value in the table, or nil if not found
Utilities.getIndexByValue = function (t, value)
    local i = 0
    for _, v in pairs(t) do
        i = i + 1
        if v == value then
            return i -- Return zero-based index
        end
    end
    return nil
end

--- Get the index of a key in a table treated as a map
--- @param t table<any, any> - The table to be searched
--- @param key any - The key to be searched
--- @return integer|nil - The zero-based index of the key in the table, or nil if not found
Utilities.getIndexByKey = function (t, key)
    local i = 0
    for k, _ in pairs(t) do
        i = i + 1
        if k == key then
            return i -- Return zero-based index
        end
    end
    return nil
end

--- Get the key of a value in a table treated as a map
--- @param t table<any, any> - The table to be searched
--- @param value any - The value to be searched
--- @return any - The key of the value in the table, or nil if not found
Utilities.getKeyByValue = function (t, value)
    for k, v in pairs(t) do
        if v == value then
            return k
        end
    end
    return nil
end

--- Create a list of strings from an enum table
---@param enumTable any - The enum table ( ex: Game.SkillType, Game.SkillMastery )
---@return table<integer, string>
function enumTableToStringList(enumTable)
    ---@cast enumTable table<string, any>
    local result = {}
    for k, _ in pairs(enumTable) do
        table.insert(result, k)
    end
    return result
end

---@class Rect
---@field x number
---@field y number
---@field w number
---@field h number

---Create a table with x,y,w,h fields
---@param x number
---@param y number
---@param w number
---@param h number
---@return Rect
Utilities.rect = function (x, y, w, h)
    return {
        x = x,
        y = y,
        w = w,
        h = h
    }
end

---@class Color
---@field r number
---@field g number
---@field b number
---@field a number

---Create a table with r,g,b,a fields
---@param r number
---@param g number
---@param b number
---@param a number
---@return Color
Utilities.color = function (r, g, b, a)
    return {
        r = r,
        g = g,
        b = b,
        a = a
    }
end

---@param t table<any, any>
---@param indent integer
---@param seen table<table<any, any>, boolean>
Utilities.printTable = function (t, indent, seen)
    indent = indent or 0
    seen = seen or {}
    if seen[t] then
        print(string.rep(" ", indent) .. "*cycle*")
        return
    end
    seen[t] = true

    for k, v in pairs(t) do
        local key = tostring(k)
        if type(v) == "table" then
            print(string.rep(" ", indent) .. key .. ":")
            Utilities.printTable(v, indent + 2, seen)
        else
            print(string.rep(" ", indent) .. key .. " = " .. tostring(v))
        end
    end
end

---@param value string|number|integer
---@return integer|nil
Utilities.tointeger = function (value)
    local result = nil
    if type(value) == "string" then
        local number = tonumber(value)
        ---@cast number integer
        result = number;
    elseif type(value) == "number" then
        ---@cast value integer
        result = value
    end
    return result
end

Utilities.canBeConvertedTo = function (value, typeName)
    if type(value) == typeName then
        return true
    end

    if typeName == "number" then
        return tonumber(value) ~= nil
    elseif typeName == "characterIndex" then
        return Utilities.tointeger(value) ~= nil
    elseif typeName == "boolean" then
        return value == "true" or value == "false"
    elseif typeName == "string" or typeName == "enum" then
        return type(value) == "string"
    end

    return false
end

return Utilities
