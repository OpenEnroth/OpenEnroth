--- MMExtension's event scripting API on top of OpenEnroth's evt interpreter.
---
--- Scripts written for MMExtension run here unchanged: `evt.map[376] = function() ... end`, `evt.Cmp("QBits", 240)`,
--- `Game.MapEvtLines:RemoveEvent(376)`, `events.LoadMap`, `Timer(...)`. They live in `scripts/maps/<map>.lua`,
--- `scripts/maps/*.<map>.lua` and `scripts/global/*.lua`, and see the API as globals of their own environment.

-- The names that scripts see are MMExtension's, and those are in pascal case.
---@diagnostic disable: name-style-check

local Bindings = require "bindings.evt"
local Log = require "bindings.log"

---@class EvtFrame One running handler.
---@field coroutine thread
---@field context EvtScriptContext
---@field eventId integer
---@field isGlobal boolean
---@field player integer|string Who the commands apply to, as `evt.ForPlayer` sets.
---@field defaultPlayer integer

---@class EvtHandler
---@field callback function
---@field scope string? "map" for handlers that go away with the map.

---@alias EvtHandlerLists table<any, EvtHandler[]>

local stopSignal = {} -- Yielded by a handler that must not continue.
local waitSignal = {} -- Yielded by a handler that continues once the engine resumes its event.

---@type EvtFrame?
local current = nil
---@type table<integer, EvtFrame>
local waiting = {}
---@type string?
local loadingScope = nil

---@param frame EvtFrame
---@param ... any Passed to the handler.
---@return boolean mapExitTriggered
---@return any result What the handler returned, if it ran to its end.
local function resume(frame, ...)
    local previous = current
    current = frame
    ---@type boolean, any
    local ok, result = coroutine.resume(frame.coroutine, ...)
    current = previous

    if not ok then
        Log.error(debug.traceback(frame.coroutine, tostring(result)))
        result = nil
    elseif result == waitSignal then
        waiting[frame.eventId] = frame
        result = nil
    elseif result == stopSignal then
        result = nil
    end
    return frame.context:isMapExitTriggered(), result
end

---@param eventId integer
---@param isGlobal boolean
---@param targetPid integer
---@param canShowMessages boolean
---@return EvtFrame
local function newFrame(eventId, isGlobal, targetPid, canShowMessages)
    local player = Bindings.defaultPlayer()
    ---@type EvtFrame
    return {
        coroutine = coroutine.running(),
        context = Bindings.newContext(eventId, targetPid, canShowMessages),
        eventId = eventId,
        isGlobal = isGlobal,
        player = player,
        defaultPlayer = player,
    }
end

---@param callback function
---@param eventId integer
---@param isGlobal boolean
---@param targetPid integer
---@param canShowMessages boolean
---@param ... any Passed to the handler.
---@return boolean mapExitTriggered
---@return any result
local function run(callback, eventId, isGlobal, targetPid, canShowMessages, ...)
    local frame = newFrame(eventId, isGlobal, targetPid, canShowMessages)
    frame.coroutine = coroutine.create(callback)
    return resume(frame, ...)
end

--- A table of handler lists. `t[key] = f` adds a handler, `t[key].last` is the handler added last, `t[key]()` runs
--- them all.
---@param runHandler fun(callback: function, key: any, ...): any How to run one handler.
---@return table events
---@return EvtHandlerLists lists
local function newEvents(runHandler)
    ---@type EvtHandlerLists
    local lists = {}

    ---@param key any
    ---@param ... any
    ---@return any result What the last handler that returned something returned.
    local function call(key, ...)
        ---@type any
        local result = nil
        for _, handler in ipairs(lists[key] or {}) do
            ---@type any
            local value = runHandler(handler.callback, key, ...)
            if value ~= nil then
                result = value
            end
        end
        return result
    end

    ---@param key any
    ---@param callback function
    local function remove(key, callback)
        local list = lists[key] or {}
        for i = #list, 1, -1 do
            if list[i].callback == callback then
                table.remove(list, i)
            end
        end
    end

    ---@type table<string, function>
    local methods = {
        exists = function (key) return lists[key] ~= nil and #lists[key] > 0 end,
        clear = function (key) lists[key] = nil end,
        remove = remove,
        call = call,
        cocall = call,
        cocalls = call,
    }

    local events = setmetatable({}, {
        ---@param _ table
        ---@param key any
        ---@return any
        __index = function (_, key)
            if methods[key] then
                return methods[key]
            end
            return setmetatable({}, {
                ---@param _ table
                ---@param field string
                ---@return any
                __index = function (_, field)
                    local list = lists[key] or {}
                    if field == "last" then
                        return list[#list] and list[#list].callback
                    elseif field == "first" then
                        return list[1] and list[1].callback
                    elseif field == "clear" then
                        return function () lists[key] = nil end
                    elseif field == "exists" then
                        return function () return #list > 0 end
                    end
                    return nil
                end,
                __call = function (_, ...) return call(key, ...) end,
            })
        end,
        ---@param _ table
        ---@param key any
        ---@param callback function?
        __newindex = function (_, key, callback)
            if callback == nil then
                lists[key] = nil
                return
            end
            lists[key] = lists[key] or {}
            table.insert(lists[key], { callback = callback, scope = loadingScope })
        end,
    })
    return events, lists
end

---@param lists EvtHandlerLists
local function removeMapHandlers(lists)
    for key, list in pairs(lists) do
        for i = #list, 1, -1 do
            if list[i].scope == "map" then
                table.remove(list, i)
            end
        end
        if #list == 0 then
            lists[key] = nil
        end
    end
end

---@type table<any, any>
local evt = {}

local mapExitTriggered = false

---@param isGlobal boolean
---@return table events
---@return EvtHandlerLists lists
local function newEventHandlers(isGlobal)
    ---@param callback function
    ---@param eventId integer
    ---@param targetPid integer?
    ---@param canShowMessages boolean?
    return newEvents(function (callback, eventId, targetPid, canShowMessages)
        local exit = run(callback, eventId, isGlobal, targetPid or 0, canShowMessages ~= false)
        mapExitTriggered = mapExitTriggered or exit
    end)
end

---@param callback function
---@param name string
---@param ... any
---@return any
local events, eventLists = newEvents(function (callback, name, ...)
    local _, result = run(callback, 0, false, 0, name ~= "LoadMap", ...)
    return result
end)
---@type EvtHandlerLists, EvtHandlerLists, EvtHandlerLists
local mapLists, globalLists, topicLists = {}, {}, {}
---@type table<any, any>, table<any, any>, table<any, any>
local mapEvents, globalEvents, topicEvents = {}, {}, {}
---@type table<integer, string>
local hints = {}
---@type table<integer, integer>
local houses = {}

local function resetGlobalHandlers()
    globalEvents, globalLists = newEventHandlers(true)
    ---@param callback function
    ---@param topic integer
    ---@return any
    topicEvents, topicLists = newEvents(function (callback, topic)
        local _, result = run(callback, topic, true, 0, false, topic)
        return result
    end)
    evt.global, evt.Global, evt.CanShowTopic = globalEvents, globalEvents, topicEvents
end

local function resetMapHandlers()
    mapEvents, mapLists = newEventHandlers(false)
    hints, houses = {}, {}
    evt.map, evt.Map = mapEvents, mapEvents
    evt.hint, evt.Hint = hints, hints
    evt.house, evt.House = houses, houses
end

---@return EvtFrame
local function currentFrame()
    return current or newFrame(0, false, 0, true)
end

---@param name string
---@param player integer|string|nil
---@param first any The arguments as a table, or the first of them.
---@param ... any The rest of the arguments.
---@return boolean? result Whether the condition held, if the command is one.
local function execute(name, player, first, ...)
    ---@type table
    local args = type(first) == "table" and first or { first, ... }

    local frame = currentFrame()
    local result, state = frame.context:execute(name, args, player or frame.player)
    if state ~= "ok" and current then
        ---@diagnostic disable-next-line: await-in-sync
        coroutine.yield(state == "wait" and waitSignal or stopSignal)
    end
    return result
end

---@type table<integer|string, integer>
evt.Players = { [0] = 0, 1, 2, 3, Current = 4, current = 4, All = 5, all = 5, Random = 6, random = 6 }

for _, name in ipairs(Bindings.commands()) do
    evt[name] = function (...) return execute(name, nil, ...) end
end
evt.Sub = evt.Subtract

--- `evt.All.Add("Exp", 1000)` and `evt[0].Add("Gold", 1000)` apply one command to the given player.
---@type table<integer|string, table>
local playerCommands = {}
for player in pairs(evt.Players) do
    playerCommands[player] = setmetatable({}, {
        ---@param _ table
        ---@param name string
        ---@return function?
        __index = function (_, name)
            if type(evt[name]) ~= "function" then
                return nil
            end
            return function (...) return execute(name, player, ...) end
        end,
    })
end

---@param player integer|string|table
---@return table evt
function evt.ForPlayer(player)
    if type(player) == "table" then
        ---@type integer|string
        player = player.Player or player[1]
    end
    currentFrame().player = player
    return evt
end

---@return boolean
function evt.InGlobal()
    return currentFrame().isGlobal
end

---@param eventId integer
---@param houseId integer
function evt.HouseDoor(eventId, houseId)
    houses[eventId] = houseId
    mapEvents[eventId] = function ()
        execute("EnterHouse", nil, houseId)
    end
end

evt.str = setmetatable({}, {
    ---@param _ table
    ---@param index integer
    ---@return string?
    __index = function (_, index) return Bindings.str(index) end,
    ---@param _ table
    ---@param index integer
    ---@param value string
    __newindex = function (_, index, value) Bindings.setStr(index, value) end,
})
evt.Str = evt.str

setmetatable(evt, {
    ---@param _ table
    ---@param key any
    ---@return any
    __index = function (_, key)
        if key == "Player" then
            return currentFrame().player
        elseif key == "CurrentPlayer" then
            return currentFrame().defaultPlayer
        end
        return playerCommands[key]
    end,
    ---@param self table
    ---@param key any
    ---@param value any
    __newindex = function (self, key, value)
        if key == "Player" then
            currentFrame().player = value
        else
            rawset(self, key, value)
        end
    end,
})

--- Ids of the evt events that the scripts removed, global ones and the current map's ones.
---@type table<boolean, integer[]>
local removedEvents = { [true] = {}, [false] = {} }

---@param isGlobal boolean
---@return table lines
local function newEvtLines(isGlobal)
    return setmetatable({
        ---@param _ table
        ---@param eventId integer
        RemoveEvent = function (_, eventId)
            Bindings.removeEvent(isGlobal, eventId)
            table.insert(removedEvents[isGlobal], eventId)
        end,
    }, {
        ---@param _ table
        ---@param key string
        ---@return integer?
        __index = function (_, key)
            if key == "Count" or key == "count" then
                return Bindings.eventCount(isGlobal)
            end
            return nil
        end,
        ---@param _ table
        ---@param key string
        ---@param value integer
        __newindex = function (_, key, value)
            if (key == "Count" or key == "count") and value == 0 then
                Bindings.clearEvents(isGlobal)
            else
                error("Only Count = 0 is supported", 2)
            end
        end,
    })
end

--- `Game.Rand() % n` draws like the interpreter's random jump does, so that a decompiled script rolls what its evt
--- file would have rolled.
local randomValue = setmetatable({}, {
    ---@param _ table
    ---@param hi integer
    ---@return integer
    __mod = function (_, hi) return Bindings.random(hi) end,
})

local Game = setmetatable({
    MapEvtLines = newEvtLines(false),
    GlobalEvtLines = newEvtLines(true),
    Rand = function () return randomValue end,
}, {
    ---@param _ table
    ---@param key string
    ---@return number?
    __index = function (_, key)
        if key == "Time" then
            return Bindings.time()
        end
        return nil
    end,
})

local Party = {
    QBits = setmetatable({}, {
        ---@param _ table
        ---@param bit integer
        ---@return boolean
        __index = function (_, bit) return Bindings.questBit(bit) end,
        ---@param _ table
        ---@param bit integer
        ---@param value any
        __newindex = function (_, bit, value) Bindings.setQuestBit(bit, value and true or false) end,
    }),
}

local Mouse = {
    Item = setmetatable({}, {
        ---@param _ table
        ---@param key string
        ---@return integer?
        __index = function (_, key)
            if key == "Number" then
                return Bindings.mouseItem()
            end
            return nil
        end,
    }),
}

---@type table<string, any>
local const = Bindings.constants()
const.Minute = 256
const.Second = const.Minute / 60
const.Hour = 60 * const.Minute
const.Day = 24 * const.Hour
const.Week = 7 * const.Day
const.Month = 4 * const.Week
const.Year = 12 * const.Month
const.Novice, const.Expert, const.Master, const.GM = 1, 2, 3, 4

---@class EvtTimer
---@field callback function
---@field period number
---@field startTime number?

---@type EvtTimer[]
local pendingTimers = {}
---@type table<function, boolean>
local removedTimers = {}

---@param callback function
---@param period number?
---@param startTime number?
local function Timer(callback, period, startTime)
    removedTimers[callback] = nil
    table.insert(pendingTimers, { callback = callback, period = period or const.Minute, startTime = startTime })
end

--- OpenEnroth doesn't fire timers on a map refill, so this is `Timer` that follows the calendar.
---@param callback function
---@param period number?
---@param startTime number?
local function RefillTimer(callback, period, startTime)
    Timer(callback, period or const.Day, startTime or 0)
end

---@param callback function
local function RemoveTimer(callback)
    removedTimers[callback] = true
end

local function registerTimers()
    for _, timer in ipairs(pendingTimers) do
        Bindings.addTimer(timer.period, timer.startTime, function ()
            if removedTimers[timer.callback] then
                return false
            end
            return (run(timer.callback, 0, false, 0, true))
        end)
    end
    pendingTimers = {}
end

---@param source table<any, any>
---@param destination table<any, any>?
---@param overwrite boolean?
---@return table<any, any> destination
local function copy(source, destination, overwrite)
    destination = destination or {}
    for key, value in pairs(source) do
        if overwrite or destination[key] == nil then
            destination[key] = value
        end
    end
    return destination
end

---@type table<string, any>
local environment = setmetatable({
    evt = evt,
    Evt = evt,
    events = events,
    Game = Game,
    Party = Party,
    Mouse = Mouse,
    const = const,
    Timer = Timer,
    RefillTimer = RefillTimer,
    RemoveTimer = RemoveTimer,
    ---@param strings table
    ---@return table
    Localize = function (strings) return strings end,
    table = setmetatable({ copy = copy }, { __index = table }),
}, { __index = _G })

---@param name string "vars" or "mapvars".
local function resetVariables(name)
    environment[name] = {}
    environment[name == "vars" and "Vars" or "MapVars"] = environment[name]
end

---@param chunk function? A loaded script.
---@param message string? Why it didn't load.
---@param scope string?
local function runScript(chunk, message, scope)
    if not chunk then
        Log.error(tostring(message))
        return
    end
    loadingScope = scope
    run(chunk, 0, false, 0, false)
    loadingScope = nil
end

---@param paths string[]
---@param scope string?
local function runScripts(paths, scope)
    for _, path in ipairs(paths) do
        local chunk, message = Bindings.loadScript(path, environment)
        runScript(chunk, message, scope)
    end
end

--- Replaces what's left of an evt file with its decompiled script, see `debug.decompiled_events`.
---@param name string
---@param isGlobal boolean
---@param scope string?
local function runDecompiledEvents(name, isGlobal, scope)
    if Bindings.isDecompilingEvents() and Bindings.eventCount(isGlobal) > 0 then
        local script = Bindings.decompile(name, removedEvents[isGlobal])
        local chunk, message = Bindings.loadString(script, name .. ".evt", environment)
        runScript(chunk, message, scope)
    end
end

local Core = { environment = environment }

function Core.loadGlobalScripts()
    waiting, pendingTimers, removedTimers = {}, {}, {}
    for key in pairs(eventLists) do
        eventLists[key] = nil
    end
    resetGlobalHandlers()
    resetMapHandlers()
    resetVariables("vars")
    removedEvents[true] = {}
    runScripts(Bindings.globalScripts(), nil)
    runDecompiledEvents("global", true, nil)
end

---@param mapName string
function Core.loadMapScripts(mapName)
    waiting, pendingTimers = {}, {}
    removeMapHandlers(eventLists)
    removeMapHandlers(globalLists)
    removeMapHandlers(topicLists)
    resetMapHandlers()
    resetVariables("mapvars")
    removedEvents[false] = {}
    runScripts(Bindings.mapScripts(mapName), "map")
    runDecompiledEvents(mapName, false, "map")
end

---@param isGlobal boolean
---@param eventId integer
---@return boolean
function Core.hasEvent(isGlobal, eventId)
    local list = (isGlobal and globalLists or mapLists)[eventId]
    return list ~= nil and #list > 0
end

---@param isGlobal boolean
---@param eventId integer
---@param targetPid integer
---@param canShowMessages boolean
---@return boolean mapExitTriggered
function Core.runEvent(isGlobal, eventId, targetPid, canShowMessages)
    mapExitTriggered = false
    local handlers = isGlobal and globalEvents or mapEvents
    handlers.call(eventId, targetPid, canShowMessages)
    return mapExitTriggered
end

---@param eventId integer
---@return boolean? mapExitTriggered Nil if no handler of the event was waiting.
function Core.resumeEvent(eventId)
    local frame = waiting[eventId]
    if not frame then
        return nil
    end
    waiting[eventId] = nil
    return (resume(frame))
end

---@param eventId integer
---@return string?
function Core.eventHint(eventId)
    local house = houses[eventId]
    return hints[eventId] or (house and Bindings.houseName(house))
end

---@param eventId integer
---@return boolean?
function Core.canShowTopic(eventId)
    ---@type any
    local result = topicEvents.call(eventId)
    if result == nil then
        return nil
    end
    return result and true or false
end

function Core.onMapLoad()
    registerTimers()
    events.call("LoadMap")
end

function Core.onMapLeave()
    events.call("LeaveMap")
end

resetGlobalHandlers()
resetMapHandlers()
resetVariables("vars")
resetVariables("mapvars")

return Core
