local InputContext = {}

---@type table<PlatformKey, table<integer, fun()>>
local keyPressedCallbacks = {}

---register a callback to the Keyboard press event
---@param key PlatformKey
---@param callback fun()
---@return function - call this function to unregister the callback
InputContext.registerKeyPress = function (key, callback)
    local callbacks = keyPressedCallbacks[key]
    if callbacks == nil then
        callbacks = {}
        keyPressedCallbacks[key] = callbacks
    end

    table.insert(callbacks, callback)
    return function ()
        for i, registeredCallback in ipairs(callbacks) do
            if registeredCallback == callback then
                table.remove(callbacks, i)
            end
        end
    end
end

---@class KeyCallbackItem
---@field key PlatformKey
---@field callback fun()

---Register a list of callbacks to keyboard press events.
---Useful in case we want a common unregister function instead of dealing with multiple
---@param items table<integer, KeyCallbackItem>
---@return fun()
InputContext.registerKeyPressBulk = function (items)
    ---@type table<integer, KeyCallbackItem>
    local unregisterBulk = {}
    for _, item in pairs(items) do
        local unregister = InputContext.registerKeyPress(item.key, item.callback)
        table.insert(unregisterBulk, unregister)
    end
    return function ()
        for _, unregister in pairs(unregisterBulk) do
            unregister()
        end
    end
end

---The _globalOnKeyPress function is a OE special function called from c++
---when a new keyPress event is triggered
---@param keyPressed PlatformKey
---@return boolean - Indicate if the input has been handled or not, in order to keep the propagation
---@private
---@diagnostic disable-next-line: name-style-check
_globalOnKeyPress = function (keyPressed)
    for key, callbacks in pairs(keyPressedCallbacks) do
        if key == keyPressed then
            for _, callback in pairs(callbacks) do
                if callback() then
                    return true
                end
            end
        end
    end
    return false
end

return InputContext
