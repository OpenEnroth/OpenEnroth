function tobool(str_value)
    return str_value == "true" or str_value == "1"
end

-- util function to convert a value to a number. in case of failure the default_value is returned
function tonumber_or(amount, default_value)
    amount = tonumber(amount)
    return amount and amount or default_value
end
