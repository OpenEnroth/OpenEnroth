local mm = require "mmbindings"

local add_item_to_inventory = function(item_id, character_index)
    if not character_index then
        character_index = mm.party.get_active_character()
    end

    local item = mm.items.get_item_info(item_id)
    if item then
        local result = mm.party.add_item_to_inventory(character_index, item_id)
        local character = mm.party.get_character_info(character_index, { "name" })
        if result then
            return character.name.." gained item: "..item.name, true
        else
            return "Can't give to "..character.name.." item: "..item_id.." - There's not enough space in the inventory", false
        end
    else
        return "Can't give item: "..item_id.." - Item does not exist.", false
    end
end

local add_random_item_to_inventory = function(character_index, filter_function)
    local item_id = mm.items.get_random_item(filter_function)
    return add_item_to_inventory(item_id, character_index)
end

local filter_item_by_min_level = function(min_level)
    return function(item_id)
        return mm.items.get_item_info(item_id).level >= min_level
    end
end

local add_random_special_item_to_inventory = function(character_index, min_level)
    return add_random_item_to_inventory(character_index, filter_item_by_min_level(min_level and min_level or 6))
end

local subcommands = {
    add = add_item_to_inventory,
    addrandom = add_random_item_to_inventory,
    addrandomspecial = add_random_special_item_to_inventory
}

return {
    name = "inventory",
    description = "Add/Remove items from character inventory.",
    details = "",
    callback = subcommands
}
