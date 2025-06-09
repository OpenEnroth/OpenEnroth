#include "Inventory.h"

#include <cassert>

#include "Engine/Snapshots/EntitySnapshots.h"
#include "Library/Snapshots/CommonSnapshots.h"
#include "Engine/Tables/ChestTable.h"
#include "Library/Logger/Logger.h"

Inventory::Inventory(Sizei gridSize, int capacity) : _gridSize(gridSize), _capacity(capacity) {
    assert(gridSize.w > 0 && gridSize.h > 0);
    assert(gridSize.w * gridSize.h <= MAX_ITEMS);
    assert(capacity > 0);
    assert(capacity <= MAX_ITEMS);
}

InventoryEntry *Inventory::gridItem(Pointi position) {
    if (!gridRect().contains(position))
        return nullptr;

    int xy = position.y * _gridSize.w + position.x;
    int index = _grid[xy];
    if (index == 0)
        return nullptr;

    InventoryEntry *result = nullptr;
    if (index > 0) {
        result = &_entries[index - 1];
    } else {
        xy = -index - 1;
        assert(xy >= 0 && xy < position.y * _gridSize.w + position.x);
        index = _grid[xy];
        assert(index > 0);
        result = &_entries[index - 1];
    }

    assert(result);
    assert(result->_item.itemId != ITEM_NULL);
    assert(result->_zone == INVENTORY_ZONE_GRID);
    return result;
}

InventoryEntry *Inventory::equippedItem(ItemSlot slot) {
    int index = _equipment[slot];
    if (index == 0)
        return nullptr;

    InventoryEntry *result = &_entries[index - 1];
    assert(result->_item.itemId != ITEM_NULL);
    assert(result->_zone == INVENTORY_ZONE_EQUIPMENT);
    return result;
}

bool Inventory::canAddGridItem(Pointi position, Sizei size) const {
    assert(size.h > 0 && size.w > 0);

    if (!gridRect().contains(Recti(position, size)))
        return false;

    if (_size >= _capacity)
        return false;

    return isGridFree(position, size);
}

InventoryEntry *Inventory::addGridItem(Pointi position, const Item &item) {
    assert(item.itemId != ITEM_NULL);
    assert(canAddGridItem(position, item.inventorySize()));

    int index = findFreeIndex();
    assert(index != -1); // Can store item => we should have free indices.

    return addGridItemAtIndex(position, item, index);
}

bool Inventory::canAddEquippedItem(ItemSlot slot) const {
    return _size < _capacity && _equipment[slot] == 0;
}

InventoryEntry *Inventory::addEquippedItem(ItemSlot slot, const Item &item) {
    assert(item.itemId != ITEM_NULL);
    assert(canAddEquippedItem(slot));

    int index = findFreeIndex();
    assert(index != -1); // Can equip item => we should have free indices.

    InventoryEntry &entry = _entries[index];
    entry._item = item;
    entry._zone = INVENTORY_ZONE_EQUIPMENT;
    entry._position = Pointi();
    entry._slot = slot;
    _size++;

    checkInvariants();
    return &entry;
}

bool Inventory::canAddHiddenItem() const {
    return _size < _capacity;
}

InventoryEntry *Inventory::addHiddenItem(const Item &item) {
    assert(item.itemId != ITEM_NULL);
    assert(canAddHiddenItem());

    int index = findFreeIndex();
    assert(index != -1);

    return addHiddenItemAtIndex(item, index);
}

Item Inventory::takeItem(InventoryEntry *entry) {
    assert(entry >= _entries.data() && entry < _entries.data() + _entries.size()); // Should be our item.
    assert(entry->_item.itemId != ITEM_NULL);

    if (entry->_zone == INVENTORY_ZONE_GRID) {
        int xy = entry->_position.y * _gridSize.w + entry->_position.x;
        assert(xy >= 0 && xy < _gridSize.w * _gridSize.h);

        Sizei itemSize = entry->_item.inventorySize();
        for (int y = 0; y < itemSize.h; y++, xy += _gridSize.w - itemSize.w)
            for (int x = 0; x < itemSize.w; x++, xy++)
                _grid[xy] = 0;
    }

    Item result = entry->_item;
    entry->_item = Item();
    entry->_zone = INVENTORY_ZONE_HIDDEN;
    entry->_position = Pointi(0, 0);
    entry->_slot = ITEM_SLOT_INVALID;
    _size--;

    checkInvariants();
    return result;
}

[[nodiscard]] std::optional<Pointi> Inventory::findGridSpace(Sizei size) const {
    assert(size.w > 0 && size.h > 0);

    if (_size >= _capacity)
        return std::nullopt;

    if (_gridSize.w < size.w || _gridSize.h < size.h)
        return std::nullopt;

    for (int x = 0, xx = _gridSize.w - size.w; x < xx; x++)
        for (int y = 0, yy = _gridSize.h - size.h; y < yy; y++)
            if (isGridFree(Pointi(x, y), size))
                return Pointi(x, y);

    return std::nullopt;
}

InventoryEntry *Inventory::findEntry(ItemId itemId) {
    for (auto &entry : _entries)
        if (entry._item.itemId == itemId)
            return &entry;
    return nullptr;
}

int Inventory::findFreeIndex() const {
    auto pos = std::ranges::find_if(_entries, [](const InventoryEntry &entry) { return entry._item.itemId == ITEM_NULL; });
    return pos == _entries.end() ? -1 : pos - _entries.begin();
}

bool Inventory::isGridFree(Pointi position, Sizei size) const {
    assert(gridRect().contains(Recti(position, size)));
    for (int y = 0, xy = position.y * _gridSize.w + position.x; y < size.h; y++, xy += _gridSize.w - size.w)
        for (int x = 0; x < size.w; x++, xy++)
            if (_grid[xy] != 0)
                return false;
    return true;
}

InventoryEntry *Inventory::addGridItemAtIndex(Pointi position, const Item &item, int index) {
    int cornerXy = position.y * _gridSize.w + position.x;
    Sizei itemSize = item.inventorySize();
    for (int y = 0, xy = cornerXy; y < itemSize.h; y++, xy += _gridSize.w - itemSize.w)
        for (int x = 0; x < itemSize.w; x++, xy++)
            _grid[xy] = -cornerXy - 1;
    _grid[cornerXy] = index + 1;

    InventoryEntry &entry = _entries[index];
    entry._item = item;
    entry._zone = INVENTORY_ZONE_GRID;
    entry._position = position;
    entry._slot = ITEM_SLOT_INVALID;
    _size++;

    checkInvariants();
    return &entry;
}

InventoryEntry *Inventory::addHiddenItemAtIndex(const Item &item, int index) {
    InventoryEntry &entry = _entries[index];
    entry._item = item;
    entry._zone = INVENTORY_ZONE_HIDDEN;
    entry._position = Pointi();
    entry._slot = ITEM_SLOT_INVALID;
    _size++;

    checkInvariants();
    return &entry;
}

void Inventory::checkInvariants() const {
#ifndef NDEBUG
    for (int xy = 0; xy < _gridSize.w * _gridSize.h; xy++) {
        if (_grid[xy] == 0)
            continue;

        if (_grid[xy] > 0) {
            int index = _grid[xy] - 1;
            assert(_entries[index]._zone == INVENTORY_ZONE_GRID);
            assert(_entries[index]._position == Pointi(xy % _gridSize.w, xy / _gridSize.w));
            assert(_entries[index]._item.itemId != ITEM_NULL);
        }

        if (_grid[xy] < 0) {
            int mainXy = -_grid[xy] - 1;
            assert(_grid[mainXy] > 0);
        }
    }
#endif
}
