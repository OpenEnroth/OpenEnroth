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
---@field scope string? "map" for a handler of the map's scripts. What it registers goes away with the map too.
---@field player integer|string Who the commands apply to, as `evt.ForPlayer` sets.
---@field defaultPlayer integer

---@class EvtRunOptions How to run a handler. All fields are optional.
---@field eventId integer?
---@field isGlobal boolean?
---@field targetPid integer?
---@field canShowMessages boolean?
---@field scope string?
---@field player integer? Who the commands apply to until the handler says otherwise.

---@class EvtHandler
---@field callback function
---@field scope string?

---@alias EvtHandlerLists table<any, EvtHandler[]>

local stopSignal = {} -- Yielded by a handler that must not continue.
local waitSignal = {} -- Yielded by a handler that continues once the dialogue it opened closes.

---@type EvtFrame?
local current = nil
---@type EvtFrame? The handler that waits for the dialogue that is open now.
local waiting = nil

--- True while the decompiled script of an evt file runs. What it registers goes before what the scripts registered,
--- the way the evt events run before the scripted handlers, and it doesn't override the scripts' hints and strings.
local decompiling = false
---@type table<table, integer>
local prepended = {}

---@param list any[]
---@return any[]
local function copyList(list)
    ---@type any[]
    local result = {}
    for i = 1, #list do
        result[i] = list[i]
    end
    return result
end

---@param list table
---@param value any
local function insert(list, value)
    if decompiling then
        prepended[list] = (prepended[list] or 0) + 1
        table.insert(list, prepended[list], value)
    else
        table.insert(list, value)
    end
end

---@param frame EvtFrame
---@param ... any Passed to the handler.
---@return boolean mapExitTriggered
---@return any result What the handler returned, if it ran to its end.
---@return boolean isWaiting Whether the handler stopped until a dialogue closes.
local function resume(frame, ...)
    local previous = current
    current = frame
    ---@type boolean, any
    local ok, result = coroutine.resume(frame.coroutine, ...)
    current = previous

    local isWaiting = false
    if not ok then
        Log.error(debug.traceback(frame.coroutine, tostring(result)))
        result = nil
    elseif result == waitSignal then
        waiting = frame
        isWaiting = true
        result = nil
    elseif result == stopSignal then
        result = nil
    end
    return frame.context:isMapExitTriggered(), result, isWaiting
end

---@param callback function
---@param options EvtRunOptions
---@param ... any Passed to the handler.
---@return boolean mapExitTriggered
---@return any result
---@return boolean isWaiting
local function run(callback, options, ...)
    local eventId = options.eventId or 0
    local player = options.player or Bindings.defaultPlayer()
    ---@type EvtFrame
    local frame = {
        coroutine = coroutine.create(callback),
        context = Bindings.newContext(eventId, options.targetPid or 0, options.canShowMessages ~= false),
        eventId = eventId,
        isGlobal = options.isGlobal or false,
        scope = options.scope,
        player = player,
        defaultPlayer = player,
    }
    return resume(frame, ...)
end

---@return EvtFrame
local function currentFrame()
    assert(current, "evt works only while a script or one of its handlers runs")
    return current
end

---@return string? scope What a handler, a hint or a timer registered now belongs to.
local function registrationScope()
    return current and current.scope
end

---@param value any
---@param what string
local function checkFunction(value, what)
    if type(value) ~= "function" then
        error(string.format("%s has to be a function, got %s", what, type(value)), 3)
    end
end

--- A table of handler lists. `t[key] = f` adds a handler, `t[key].last` is the handler added last, `t[key]()` runs
--- them all and `t[key].clear()` removes them.
---@param runHandler fun(handler: EvtHandler, key: any, ...): boolean, any How to run a handler. Returns whether it
---                                                                          waits for a dialogue, and its result.
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
        for _, handler in ipairs(copyList(lists[key] or {})) do
            local isWaiting, value = runHandler(handler, key, ...)
            if value ~= nil then
                result = value
            end
            if isWaiting then
                break -- Like an evt event, the rest of the handlers stop at the dialogue.
            end
        end
        return result
    end

    local events = setmetatable({}, {
        ---@param _ table
        ---@param key any
        ---@return table
        __index = function (_, key)
            return setmetatable({}, {
                ---@param _ table
                ---@param field string
                ---@return any
                __index = function (_, field)
                    local list = lists[key] or {}
                    if field == "last" then
                        return list[#list] and list[#list].callback
                    elseif field == "clear" then
                        return function () lists[key] = nil end
                    end
                    return nil
                end,
                __call = function (_, ...) return call(key, ...) end,
            })
        end,
        ---@param _ table
        ---@param key any
        ---@param callback function
        __newindex = function (_, key, callback)
            checkFunction(callback, "A handler")
            lists[key] = lists[key] or {}
            insert(lists[key], { callback = callback, scope = registrationScope() })
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
    ---@param handler EvtHandler
    ---@param eventId integer
    ---@param targetPid integer?
    ---@param canShowMessages boolean?
    ---@return boolean isWaiting
    ---@return any result
    return newEvents(function (handler, eventId, targetPid, canShowMessages)
        local options = { eventId = eventId, isGlobal = isGlobal, targetPid = targetPid, canShowMessages = canShowMessages }
        options.scope = handler.scope
        local exit, result, isWaiting = run(handler.callback, options)
        mapExitTriggered = mapExitTriggered or exit
        return isWaiting, result
    end)
end

---@param handler EvtHandler
---@param name string
---@param ... any
---@return boolean isWaiting
---@return any result
local events, eventLists = newEvents(function (handler, name, ...)
    local canShowMessages = name ~= "LoadMap" and name ~= "AfterLoadMap"
    local exit, result, isWaiting = run(handler.callback, { canShowMessages = canShowMessages, scope = handler.scope }, ...)
    mapExitTriggered = mapExitTriggered or exit
    return isWaiting, result
end)

---@type EvtHandlerLists, EvtHandlerLists, EvtHandlerLists
local mapLists, globalLists, topicLists = {}, {}, {}
---@type table<any, any>, table<any, any>, table<any, any>
local mapEvents, globalEvents, topicEvents = {}, {}, {}
---@type table<integer, string>
local hints = {}
---@type table<integer, integer>
local houses = {}

--- A table that the scripts set event hints or houses in. The decompiled script doesn't override them.
---@param values table<integer, any>
---@return table
local function newEventValues(values)
    return setmetatable({}, {
        __index = values,
        ---@param _ table
        ---@param eventId integer
        ---@param value any
        __newindex = function (_, eventId, value)
            if not decompiling or values[eventId] == nil then
                values[eventId] = value
            end
        end,
    })
end

---@type table<integer, string>, table<integer, integer>
local hintValues, houseValues = newEventValues(hints), newEventValues(houses)
evt.hint, evt.Hint, evt.house = hintValues, hintValues, houseValues

local function resetGlobalHandlers()
    globalEvents, globalLists = newEventHandlers(true)
    ---@param handler EvtHandler
    ---@param topic integer
    ---@return boolean isWaiting
    ---@return any result
    topicEvents, topicLists = newEvents(function (handler, topic)
        -- The interpreter checks the whole party for a topic.
        local options = { eventId = topic, isGlobal = true, canShowMessages = false, player = evt.Players.All }
        options.scope = handler.scope
        local _, result, isWaiting = run(handler.callback, options, topic)
        return isWaiting, result
    end)
    evt.global, evt.CanShowTopic = globalEvents, topicEvents
end

local function resetMapHandlers()
    mapEvents, mapLists = newEventHandlers(false)
    evt.map, evt.Map = mapEvents, mapEvents
    for key in pairs(hints) do
        hints[key] = nil
    end
    for key in pairs(houses) do
        houses[key] = nil
    end
end

---@param first any The arguments as a table, or the first of them.
---@param ... any The rest of the arguments.
---@return table
local function argumentTable(first, ...)
    if type(first) == "table" then
        return first
    end
    return { first, ... }
end

---@param name string
---@param player integer|string|nil
---@param ... any The arguments as a table, or one by one.
---@return boolean? result Whether the condition held, if the command is one.
local function execute(name, player, ...)
    local frame = currentFrame()
    local result, state = frame.context:execute(name, argumentTable(...), player or frame.player)
    if state ~= "ok" then
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

---@param ... any The player, or a table with it as `Player` or first.
---@return table evt
function evt.ForPlayer(...)
    ---@type table<any, integer|string>
    local args = argumentTable(...)
    local player = args.Player
    if player == nil then
        player = args[1]
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
    houseValues[eventId] = houseId
    mapEvents[eventId] = function ()
        execute("EnterHouse", nil, houseId)
    end
end

--- Indices of the strings that the scripts changed.
---@type table<integer, boolean>
local changedStrings = {}

evt.str = setmetatable({}, {
    ---@param _ table
    ---@param index integer
    ---@return string?
    __index = function (_, index) return Bindings.str(index) end,
    ---@param _ table
    ---@param index integer
    ---@param value string
    __newindex = function (_, index, value)
        if decompiling and changedStrings[index] then
            return
        end
        changedStrings[index] = changedStrings[index] or not decompiling
        Bindings.setStr(index, value)
    end,
})

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
        elseif key == "CurrentPlayer" then
            error("evt.CurrentPlayer can't be set, it's the player that was active when the handler started", 2)
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

local function onlyModulus()
    error("Game.Rand() works only as Game.Rand() % n", 2)
end

--- `Game.Rand() % n` draws like the interpreter's random jump does, so that a decompiled script rolls what its evt
--- file would have rolled. The draw waits for the modulus, and each modulus is drawn once.
---@return table
local function newRandomValue()
    ---@type table<integer, integer>
    local values = {}
    return setmetatable({}, {
        ---@param _ table
        ---@param hi integer
        ---@return integer
        __mod = function (_, hi)
            values[hi] = values[hi] or Bindings.random(hi)
            return values[hi]
        end,
        __eq = onlyModulus,
        __lt = onlyModulus,
        __le = onlyModulus,
    })
end

local Game = setmetatable({
    MapEvtLines = newEvtLines(false),
    GlobalEvtLines = newEvtLines(true),
    Rand = newRandomValue,
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

--- Makes reading a name that the table doesn't have an error, so that a typo doesn't pass as nil or zero.
---@param values table
---@param name string
---@return table
local function strict(values, name)
    return setmetatable(values, {
        ---@param _ table
        ---@param key any
        __index = function (_, key)
            error(string.format("%s.%s doesn't exist", name, tostring(key)), 2)
        end,
    })
end

---@type table<string, any>
local const = Bindings.constants()
for group, constants in pairs(const) do
    strict(constants, "const." .. group)
end
const.Minute = 256
const.Hour = 60 * const.Minute
const.Day = 24 * const.Hour
const.Week = 7 * const.Day
const.Month = 4 * const.Week
const.Year = 12 * const.Month
const.Novice, const.Expert, const.Master, const.GM = 1, 2, 3, 4
strict(const, "const")

---@class EvtTimer
---@field callback function
---@field period number
---@field startTime number?
---@field isRefill boolean `RefillTimer`, which the engine checks after every `Timer`.
---@field scope string?
---@field handle integer? The engine's handle, once the engine has the timer.

---@type EvtTimer[] The timers of the scripts, in the order they were set.
local timers = {}
---@type EvtTimer?
local runningTimer = nil
local isLevelLoaded = false -- Timers set before a level loads wait for it, the engine counts them from the level's time.
local isMapRunning = false -- Timers that fire at once and are set while a map loads fire once it has loaded.
---@type EvtTimer[]
local firstFires = {}

---@param timer EvtTimer
---@return boolean mapExitTriggered
local function fire(timer)
    local previous = runningTimer
    runningTimer = timer
    local exit = run(timer.callback, { scope = timer.scope })
    runningTimer = previous
    return exit
end

---@param timer EvtTimer
local function registerTimer(timer)
    timer.handle = Bindings.addTimer(timer.period, timer.startTime, timer.isRefill, timer.scope == nil, function ()
        return fire(timer)
    end)
end

---@param callback function
---@param period number
---@param startTime number|boolean?
---@param isRefill boolean
local function newTimer(callback, period, startTime, isRefill)
    checkFunction(callback, "A timer")
    local firesAtOnce = startTime == true
    if type(startTime) == "boolean" then
        startTime = nil
    end
    ---@cast startTime number?

    local ok, message = pcall(Bindings.checkTimer, period, startTime, isRefill)
    if not ok then
        error(message, 3)
    end

    ---@type EvtTimer
    local timer = { callback = callback, period = period, startTime = startTime, isRefill = isRefill, scope = registrationScope() }
    table.insert(timers, timer)
    if isLevelLoaded then
        registerTimer(timer)
    end
    if firesAtOnce and isMapRunning then
        mapExitTriggered = fire(timer) or mapExitTriggered
    elseif firesAtOnce then
        table.insert(firstFires, timer)
    end
end

---@param callback function
---@param period number?
---@param startTime number|boolean? Time of day to fire at, or true to also fire right away.
local function Timer(callback, period, startTime)
    newTimer(callback, period or const.Minute, startTime, false)
end

--- OpenEnroth doesn't fire timers on a map refill, so this is a `Timer` that the engine checks after the others, like
--- an evt `OnLongTimer`. Without a start time a calendar period fires at midnight.
---@param callback function
---@param period number?
---@param startTime number|boolean?
local function RefillTimer(callback, period, startTime)
    newTimer(callback, period or const.Day, startTime, true)
end

---@param callback function? The function of the timers to remove, the timer that is firing if not given.
local function RemoveTimer(callback)
    for i = #timers, 1, -1 do
        local timer = timers[i]
        if (callback == nil and timer == runningTimer) or (callback ~= nil and timer.callback == callback) then
            if timer.handle then
                Bindings.removeTimer(timer.handle)
            end
            table.remove(timers, i)
        end
    end
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
    run(chunk, { scope = scope, canShowMessages = false })
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
    if not Bindings.isDecompilingEvents() or Bindings.eventCount(isGlobal) == 0 then
        return
    end
    local script = Bindings.decompile(name, removedEvents[isGlobal])
    local chunk, message = Bindings.loadString(script, name .. ".evt", environment)
    decompiling, prepended = true, {}
    runScript(chunk, message, scope)
    decompiling = false
end

local wasInGame = false

local Core = {}

function Core.loadGlobalScripts()
    waiting, timers, firstFires, isLevelLoaded, isMapRunning, wasInGame = nil, {}, {}, false, false, false
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
    waiting, isMapRunning = nil, false
    for i = #timers, 1, -1 do
        if timers[i].scope == "map" then
            table.remove(timers, i) -- The engine dropped it with the map.
        elseif not isLevelLoaded then
            registerTimer(timers[i])
        end
    end
    isLevelLoaded = true
    removeMapHandlers(eventLists)
    removeMapHandlers(globalLists)
    removeMapHandlers(topicLists)
    resetMapHandlers()
    resetVariables("mapvars")
    removedEvents[false], changedStrings = {}, {}
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
    handlers[eventId](targetPid, canShowMessages)
    return mapExitTriggered
end

---@return boolean mapExitTriggered
function Core.resumeEvent()
    local frame = waiting
    if not frame then
        return false
    end
    waiting = nil
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
    local result = topicEvents[eventId]()
    if result == nil then
        return nil
    end
    return result and true or false
end

---@return boolean mapExitTriggered
function Core.onMapLoad()
    mapExitTriggered, isMapRunning = false, true
    for _, timer in ipairs(firstFires) do
        mapExitTriggered = fire(timer) or mapExitTriggered
    end
    firstFires = {}
    events.LoadMap(wasInGame)
    events.AfterLoadMap(wasInGame)
    wasInGame = true
    return mapExitTriggered
end

function Core.onMapLeave()
    isMapRunning = false
    events.LeaveMap()
end

resetGlobalHandlers()
resetMapHandlers()
resetVariables("vars")
resetVariables("mapvars")

return Core
