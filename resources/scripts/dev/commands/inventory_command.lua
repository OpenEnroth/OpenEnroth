local Game = require "bindings.game"
local CommandUtilities = require "dev.commands.command_utils"

local addItemToInventory = function (itemId, characterIndex)
    characterIndex = CommandUtilities.characterOrCurrent(characterIndex)

    local item = Game.items.getItemInfo(itemId)
    if item then
        local result = Game.party.addItemToInventory(characterIndex, itemId)
        local character = Game.party.getCharacterInfo(characterIndex, { "name" })
        if result then
            return character.name .. " gained item: " .. item.name, true
        else
            return "Can't give to " .. character.name ..
                " item: " .. itemId .. " - There's not enough space in the inventory", false
        end
    else
        return "Can't give item: " .. itemId .. " - Item does not exist.", false
    end
end

--- @type table<string, integer>|nil
local itemsByName = nil

--- Item names are localized, so this map can only be built once the item table is loaded. Names are not unique
--- either - two different items are both called "Lich Jar" - so colliding names get their item id appended.
--- @return table<string, integer>
local function itemNameToIdMap()
    if itemsByName then
        return itemsByName
    end

    local allItems = Game.items.allItems()
    --- Walked in id order so that the lowest id is the one that keeps the plain name. Without the sort the winner
    --- would depend on hash order, which LuaJIT reseeds on every launch.
    --- @type table<integer, integer>
    local ids = {}
    for id in pairs(allItems) do
        table.insert(ids, id)
    end
    table.sort(ids)

    itemsByName = {}
    for _, id in ipairs(ids) do
        local name = string.gsub(allItems[id], " ", "_")
        if itemsByName[name] then
            name = name .. "_" .. id
        end
        itemsByName[name] = id
    end
    return itemsByName
end

local addItemToInventoryByName = function (itemName, characterIndex)
    local itemId = stringToEnum(itemNameToIdMap(), itemName)
    return addItemToInventory(itemId, characterIndex)
end

local addRandomItemToInventory = function (characterIndex, filterFunction)
    local itemId = Game.items.getRandomItem(filterFunction)
    return addItemToInventory(itemId, characterIndex)
end

local filterItemByMinLevel = function (minLevel)
    return function (itemId)
        return Game.items.getItemInfo(itemId).level >= minLevel
    end
end

local addRandomSpecialItemToInventory = function (characterIndex, minLevel)
    return addRandomItemToInventory(characterIndex, filterItemByMinLevel(minLevel and minLevel or 6))
end

local subCommands = {
    {
        name = "add",
        callback = addItemToInventoryByName,
        params = {
            {
                name = "itemId",
                type = "enum",
                enumValues = itemNameToIdMap,
                description = "Item ID to add to the inventory."
            },
            { name = "char", type = "characterIndex", description = "Character index to add the item to. Defaults to active character." }
        },
        description = "Adds the specified item to the character's inventory."
    },
    {
        name = "addrandom",
        callback = addRandomItemToInventory,
        params = {
            { name = "char", type = "characterIndex", description = "Character index to add the random item to. Defaults to active character." }
        },
        description = "Adds a random item to the character's inventory."
    },
    {
        name = "addrandomspecial",
        callback = addRandomSpecialItemToInventory,
        params = {
            { name = "char",     type = "characterIndex", description = "Character index to add the random special item to. Defaults to active character." },
            { name = "minLevel", type = "number",         description = "Minimum level of the special item." }
        },
        description = "Adds a random special item (level >= 6) to the character's inventory."
    },
}

return {
    name = "inventory",
    description = "Add/Remove items from character inventory.",
    details = "",
    subCommands = subCommands
}
