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
    add = addItemToInventory,
    addrandom = addRandomItemToInventory,
    addrandomspecial = addRandomSpecialItemToInventory
}

return {
    name = "inventory",
    description = "Add/Remove items from character inventory.",
    details = "",
    callback = subCommands
}
