local utils = {}

---Convert a string to a boolean
---@param value string
---@return boolean
utils.tobool = function(value)
    return value ~= "false" and value ~= "0"
end

---Convert a variable to a number. in case of failure the default_value is returned
---@param amount any
---@param default_value number
---@return number
utils.tonumber_or = function(amount, default_value)
    amount = tonumber(amount)
    return amount and amount or default_value
end

--- Tell if a string is empty or nil
---@param s string
---@return boolean
utils.isempty = function(s)
    return s == nil or s == ''
end

--- Split the string according to the separator. Uses regex identifier
---@param str string - base string
---@param separator string - separator to be used
---@return table - list of splitted strings
utils.split_string = function(str, separator)
    local result = {}
    for value in str:gmatch("([^"..separator.."]+)") do
        table.insert(result, value)
    end

    return result
end

--- Take an enum table and returns the string representation of the value provided
--- It basically returns the key of the corresponding row
---
--- Example:
--- mm.SkillType = {
---    Club = 0,
---    Sword = 1,
---    Dagger = 2,
--- }
--- local result = enum_to_string(mm.SkillType, 2)
--- The value of variable "result" is "Dagger" 
---
---@param enum_table table - The enum table ( ex: mm.SkillType, mm.SkillMastery )
---@param value_to_convert any - The input is the enumeration value
---@return string
utils.enum_to_string = function(enum_table, value_to_convert)
    for k, v in pairs(enum_table) do
        if v == value_to_convert then
            return k
        end
    end

    return ""
end

return utils
