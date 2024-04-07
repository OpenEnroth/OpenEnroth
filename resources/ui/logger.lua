local callbacks = {}

logsink = function(level, message)
    for key, callback in pairs(callbacks) do
        callback(level, message)
    end
end

return {
    register = function(callback)
        table.insert(callbacks, callback)
    end
}
