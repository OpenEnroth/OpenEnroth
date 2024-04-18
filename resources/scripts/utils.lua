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

utils.isempty = function(s)
    return s == nil or s == ''
end

utils.split_string = function(str, separator)
    local result = {}
    for value in str:gmatch("([^"..separator.."]+)") do
        table.insert(result, value)
    end

    return result
end

utils.enum_to_string = function(enum_table, value_to_convert)
    for k, v in pairs(enum_table) do
        if v == value_to_convert then
            return k
        end
    end

    return ""
end

return utils