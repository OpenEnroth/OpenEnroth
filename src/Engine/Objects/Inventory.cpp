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

InventoryEntry Inventory::entry(Pointi position) {
    if (!gridRect().contains(position))
        return {};

    int xy = position.y * _gridSize.w + position.x;
    int index = _grid[xy];
    if (index == 0)
        return {};

    if (index < 0) {
        xy = -index - 1;
        assert(xy >= 0 && xy < position.y * _gridSize.w + position.x);
        index = _grid[xy];
        assert(index > 0);
    }

    index--;
    assert(_records[index].item.itemId != ITEM_NULL);
    assert(_records[index].zone == INVENTORY_ZONE_GRID);
    return InventoryEntry(this, index);
}

InventoryEntry Inventory::entry(ItemSlot slot) {
    int index = _equipment[slot];
    if (index == 0)
        return {};

    index--;
    assert(_records[index].item.itemId != ITEM_NULL);
    assert(_records[index].zone == INVENTORY_ZONE_EQUIPMENT);
    return InventoryEntry(this, index);
}

InventoryEntry Inventory::entry(int index) {
    if (index < 0 || index >= _capacity || _records[index].item.itemId == ITEM_NULL)
        return {};
    return InventoryEntry(this, index);
}

bool Inventory::canAdd(Pointi position, Sizei size) const {
    assert(size.h > 0 && size.w > 0);

    if (!gridRect().contains(Recti(position, size)))
        return false;

    if (_size >= _capacity)
        return false;

    return isGridFree(position, size);
}

bool Inventory::canAdd(Sizei size) const {
    return !!findSpace(size);
}

InventoryEntry Inventory::add(Pointi position, const Item &item) {
    assert(item.itemId != ITEM_NULL);
    assert(canAdd(position, item));

    int index = findFreeIndex();
    assert(index != -1); // Can store item => we should have free indices.

    return addAt(position, item, index);
}

InventoryEntry Inventory::add(const Item &item) {
    std::optional<Pointi> position = findSpace(item);
    assert(position);
    return add(*position, item);
}

InventoryEntry Inventory::tryAdd(Pointi position, const Item &item) {
    return canAdd(position, item) ? add(position, item) : InventoryEntry();
}

InventoryEntry Inventory::tryAdd(const Item &item) {
    std::optional<Pointi> position = findSpace(item);
    return position ? add(*position, item) : InventoryEntry();
}

bool Inventory::canEquip(ItemSlot slot) const {
    return _size < _capacity && _equipment[slot] == 0;
}

InventoryEntry Inventory::equip(ItemSlot slot, const Item &item) {
    assert(item.itemId != ITEM_NULL);
    assert(canEquip(slot));

    int index = findFreeIndex();
    assert(index != -1); // Can equip item => we should have free indices.

    return equipAt(slot, item, index);
}

InventoryEntry Inventory::tryEquip(ItemSlot slot, const Item &item) {
    return canEquip(slot) ? equip(slot, item) : InventoryEntry();
}

bool Inventory::canStash() const {
    return _size < _capacity;
}

InventoryEntry Inventory::stash(const Item &item) {
    assert(item.itemId != ITEM_NULL);
    assert(canStash());

    int index = findFreeIndex();
    assert(index != -1);

    return stashAt(item, index);
}

InventoryEntry Inventory::tryStash(const Item &item) {
    return canStash() ? stash(item) : InventoryEntry();
}

Item Inventory::take(InventoryEntry entry) {
    assert(entry);
    assert(entry._inventory == this);

    if (entry.zone() == INVENTORY_ZONE_GRID) {
        Recti geometry = entry.geometry();
        int xy = geometry.y * _gridSize.w + geometry.x;
        assert(xy >= 0 && xy < _gridSize.w * _gridSize.h);

        for (int y = 0; y < geometry.h; y++, xy += _gridSize.w - geometry.w)
            for (int x = 0; x < geometry.w; x++, xy++)
                _grid[xy] = 0;
    } else if (entry.zone() == INVENTORY_ZONE_EQUIPMENT) {
        _equipment[entry.slot()] = 0;
    }

    Item result = *entry;
    _records[entry.index()] = {};
    _size--;

    checkInvariants();
    return result;
}

std::optional<Pointi> Inventory::findSpace(Sizei size) const {
    assert(size.w > 0 && size.h > 0);

    if (_size >= _capacity)
        return std::nullopt;

    if (_gridSize.w < size.w || _gridSize.h < size.h)
        return std::nullopt;

    for (int x = 0, xx = _gridSize.w - size.w + 1; x < xx; x++)
        for (int y = 0, yy = _gridSize.h - size.h + 1; y < yy; y++)
            if (isGridFree(Pointi(x, y), size))
                return Pointi(x, y);

    return std::nullopt;
}

InventoryEntry Inventory::find(ItemId itemId) {
    for (int i = 0; i < _capacity; i++)
        if (_records[i].item.itemId == itemId)
            return InventoryEntry(this, i);
    return {};
}

void Inventory::clear() {
    _size = 0;
    _records.fill(InventoryRecord());
    _grid.fill(0);
    _equipment.fill(0);
    checkInvariants();
}

int Inventory::findFreeIndex() const {
    auto pos = std::ranges::find_if(availableRecords(), [](const InventoryRecord &data) { return data.item.itemId == ITEM_NULL; });
    return pos == availableRecords().end() ? -1 : pos - availableRecords().begin();
}

bool Inventory::isGridFree(Pointi position, Sizei size) const {
    assert(gridRect().contains(Recti(position, size)));
    for (int y = 0, xy = position.y * _gridSize.w + position.x; y < size.h; y++, xy += _gridSize.w - size.w)
        for (int x = 0; x < size.w; x++, xy++)
            if (_grid[xy] != 0)
                return false;
    return true;
}

InventoryEntry Inventory::addAt(Pointi position, const Item &item, int index) {
    int cornerXy = position.y * _gridSize.w + position.x;
    Sizei itemSize = item.inventorySize();
    for (int y = 0, xy = cornerXy; y < itemSize.h; y++, xy += _gridSize.w - itemSize.w)
        for (int x = 0; x < itemSize.w; x++, xy++)
            _grid[xy] = -cornerXy - 1;
    _grid[cornerXy] = index + 1;

    InventoryRecord &record = _records[index];
    record.item = item;
    record.zone = INVENTORY_ZONE_GRID;
    record.position = position;
    record.slot = ITEM_SLOT_INVALID;
    _size++;

    checkInvariants();
    return InventoryEntry(this, index);
}

InventoryEntry Inventory::equipAt(ItemSlot slot, const Item &item, int index) {
    _equipment[slot] = index + 1;

    InventoryRecord &record = _records[index];
    record.item = item;
    record.zone = INVENTORY_ZONE_EQUIPMENT;
    record.position = Pointi();
    record.slot = slot;
    _size++;

    checkInvariants();
    return InventoryEntry(this, index);
}

InventoryEntry Inventory::stashAt(const Item &item, int index) {
    InventoryRecord &record = _records[index];
    record.item = item;
    record.zone = INVENTORY_ZONE_STASH;
    record.position = Pointi();
    record.slot = ITEM_SLOT_INVALID;
    _size++;

    checkInvariants();
    return InventoryEntry(this, index);
}

void Inventory::checkInvariants() const {
#ifndef NDEBUG
    // Check size.
    int actualSize = 0;
    for (int i = 0; i < MAX_ITEMS; i++)
        if (_records[i].item.itemId != ITEM_NULL)
            actualSize++;
    assert(_size == actualSize);

    // Check that we don't touch tail entries.
    for (int i = _capacity; i < MAX_ITEMS; i++)
        assert(_records[i].item.itemId == ITEM_NULL);

    // Check that grid looks valid.
    for (int xy = 0; xy < _gridSize.w * _gridSize.h; xy++) {
        if (_grid[xy] == 0)
            continue;

        if (_grid[xy] > 0) {
            int index = _grid[xy] - 1;
            assert(_records[index].zone == INVENTORY_ZONE_GRID);
            assert(_records[index].position == Pointi(xy % _gridSize.w, xy / _gridSize.w));
            assert(_records[index].item.itemId != ITEM_NULL);
        }

        if (_grid[xy] < 0) {
            int mainXy = -_grid[xy] - 1;
            assert(_grid[mainXy] > 0);
        }
    }

    // Check that equipment looks valid.
    for (ItemSlot i : allItemSlots()) {
        if (_equipment[i] == 0)
            continue;
        int index = _equipment[i] - 1;
        assert(_records[index].zone == INVENTORY_ZONE_EQUIPMENT);
        assert(_records[index].item.itemId != ITEM_NULL);
    }

    // Check that ITEM_NULL entries have INVENTORY_ZONE_INVALID. We check tail entries too.
    for (int i = 0; i < MAX_ITEMS; i++)
        if (_records[i].item.itemId == ITEM_NULL)
            assert(_records[i].zone == INVENTORY_ZONE_INVALID);
#endif
}
