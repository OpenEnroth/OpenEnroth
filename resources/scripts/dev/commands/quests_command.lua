local Game = require "bindings.game"
require "utils"

local function listArcomage()
    --- @type { id: integer, flag: boolean, name: string }[]
    local entries = {}
    for id, flag in pairs(Game.party.arcomageWins()) do
        local label = enumToString(Game.ArcomageTaverns, id) or "?"
        local name = string.gsub(label, "(%l)(%u)", "%1 %2")
        entries[#entries + 1] = { id = id, flag = flag, name = name }
    end
    table.sort(entries, function (a, b) return a.name < b.name end)

    --- @type string[]
    local lines = {}
    for _, e in ipairs(entries) do
        lines[#lines + 1] = e.id .. ": " .. tostring(e.flag) .. " in " .. e.name
    end
    return table.concat(lines, "\n"), true
end

local subCommands = {
    {
        name = "arcomage",
        callback = listArcomage,
        description = "Show Arcomage win flags by tavern location."
    },
}

return {
    name = "quests",
    description = "Dev commands related to Quests.",
    details = "",
    subCommands = subCommands
}
