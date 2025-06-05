local Game = require "bindings.game"

local addItemToInventory = function (itemId, characterIndex)
    if not characterIndex then
        characterIndex = Game.party.getActiveCharacter()
    end

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

local addItemToInventoryByName = function (itemName, characterIndex)
    local itemId = stringToEnum(Game.ItemType, itemName)
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
                enumValues = Game.ItemType,
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
