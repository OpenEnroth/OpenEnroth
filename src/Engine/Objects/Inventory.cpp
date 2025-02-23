#include "Inventory.h"

#include <cassert>

#include "Engine/Snapshots/EntitySnapshots.h"
#include "Library/Snapshots/CommonSnapshots.h"

Inventory::Inventory(Sizei storageSize, int capacity) : _storageSize(storageSize), _capacity(capacity) {
    assert(storageSize.w > 0 && storageSize.h > 0);
    assert(storageSize.w * storageSize.h <= MAX_ITEMS);
    assert(capacity > 0);
    assert(capacity <= MAX_ITEMS);

    // Check that we were properly zero-initialized.
    assert(_slots[0] == ITEM_SLOT_INVALID);
    assert(_locations[0] == INVENTORY_BURIED);
}

InventoryLocation Inventory::location(const Item *item) const {
    assert(item >= _items.data() && item < _items.data() + _items.size()); // Should be our item.
    assert(item->itemId != ITEM_NULL);

    int index = item - _items.data();
    return _locations[index];
}

Recti Inventory::geometry(const Item *item) const {
    assert(item >= _items.data() && item < _items.data() + _items.size()); // Should be our item.
    assert(item->itemId != ITEM_NULL);

    int index = item - _items.data();
    return _locations[index] != INVENTORY_STORED ? Recti() : Recti(_positions[index], item->inventorySize());
}

ItemSlot Inventory::slot(const Item *item) const {
    assert(item >= _items.data() && item < _items.data() + _items.size()); // Should be our item.
    assert(item->itemId != ITEM_NULL);

    int index = item - _items.data();
    return _locations[index] != INVENTORY_EQUIPPED ? ITEM_SLOT_INVALID : _slots[index];
}

Item *Inventory::storedItem(Pointi position) {
    assert(position.x >= 0 && position.x < _storageSize.w && position.y >= 0 && position.y < _storageSize.h);

    int xy = position.y * _storageSize.w + position.x;
    int index = _storage[xy];
    if (index == 0)
        return nullptr;

    Item *result = nullptr;
    if (index > 0) {
        result = &_items[index - 1];
    } else {
        xy = -index - 1;
        assert(xy >= 0 && xy < position.y * _storageSize.w + position.x);
        index = _storage[xy];
        assert(index > 0);
        result = &_items[index - 1];
    }

    assert(result && result->itemId != ITEM_NULL);
    return result;
}

Item *Inventory::equippedItem(ItemSlot slot) {
    int index = _equipment[slot];
    if (index == 0)
        return nullptr;

    Item *result = &_items[index - 1];
    assert(result->itemId != ITEM_NULL);
    return result;
}

bool Inventory::canStoreItem(Pointi position, Sizei size) const {
    assert(size.h > 0 && size.w > 0);

    if (!storageRect().contains(Recti(position, size)))
        return false;

    if (_size >= _capacity)
        return false;

    return isStorageFree(position, size);
}

Item *Inventory::storeItem(Pointi position, const Item &item) {
    assert(item.itemId != ITEM_NULL);
    assert(canStoreItem(position, item.inventorySize()));

    int index = findFreeIndex();
    assert(index != -1); // Can store item => we should have free indices.

    int cornerXy = position.y * _storageSize.w + position.x;
    Sizei itemSize = item.inventorySize();
    for (int i = 0, xy = cornerXy; i < itemSize.w; i++, xy += _storageSize.w)
        for (int j = 0; j < itemSize.h; j++, xy++)
            _storage[xy] = -cornerXy - 1;
    _storage[cornerXy] = index + 1;

    _items[index] = item;
    _locations[index] = INVENTORY_STORED;
    _positions[index] = position;
    _slots[index] = ITEM_SLOT_INVALID;
    _size++;
    return &_items[index];
}

bool Inventory::canEquipItem(ItemSlot slot) const {
    return _size < _capacity && _equipment[slot] == 0;
}

Item *Inventory::equipItem(ItemSlot slot, const Item &item) {
    assert(item.itemId != ITEM_NULL);
    assert(canEquipItem(slot));

    int index = findFreeIndex();
    assert(index != -1); // Can equip item => we should have free indices.

    _items[index] = item;
    _locations[index] = INVENTORY_EQUIPPED;
    _positions[index] = Pointi();
    _slots[index] = slot;
    _size++;
    return &_items[index];
}

bool Inventory::canBuryItem() const {
    return _size < _capacity;
}

Item *Inventory::buryItem(const Item &item) {
    assert(item.itemId != ITEM_NULL);
    assert(canBuryItem());

    int index = findFreeIndex();
    assert(index != -1);

    _items[index] = item;
    _locations[index] = INVENTORY_BURIED;
    _positions[index] = Pointi();
    _slots[index] = ITEM_SLOT_INVALID;
    _size++;
    return &_items[index];
}

Item Inventory::takeItem(Item *item) {
    assert(item >= _items.data() && item < _items.data() + _items.size()); // Should be our item.
    assert(item->itemId != ITEM_NULL);

    int index = item - _items.data();

    if (_locations[index] == INVENTORY_STORED) {
        int xy = _positions[index].y * _storageSize.w + _positions[index].x;
        assert(xy >= 0 && xy < _storageSize.w * _storageSize.h);

        Sizei itemSize = item->inventorySize();
        for (int x = 0; x < itemSize.w; x++, xy += _storageSize.w)
            for (int y = 0; y < itemSize.h; y++, xy++)
                _storage[xy] = 0;
    }

    Item result = *item;
    _items[index] = Item();
    _locations[index] = INVENTORY_BURIED;
    _positions[index] = Pointi(0, 0);
    _slots[index] = ITEM_SLOT_INVALID;
    _size--;
    return result;
}

[[nodiscard]] std::optional<Pointi> Inventory::findStorage(Sizei size) const {
    assert(size.w > 0 && size.h > 0);

    if (_size >= _capacity)
        return std::nullopt;

    if (_storageSize.w < size.w || _storageSize.h < size.h)
        return std::nullopt;

    for (int x = 0, xx = _storageSize.w - size.w; x < xx; x++)
        for (int y = 0, yy = _storageSize.h - size.h; y < yy; y++)
            if (isStorageFree(Pointi(x, y), size))
                return Pointi(x, y);

    return std::nullopt;
}

int Inventory::findFreeIndex() const {
    auto pos = std::ranges::find_if(_items, [](const Item &item) { return item.itemId == ITEM_NULL; });
    return pos == _items.end() ? -1 : pos - _items.begin();
}

bool Inventory::isStorageFree(Pointi position, Sizei size) const {
    assert(storageRect().contains(Recti(position, size)));
    for (int x = 0, xy = position.y * _storageSize.w + position.x; x < size.w; x++, xy += _storageSize.w)
        for (int y = 0; y < size.h; y++, xy++)
            if (_storage[xy] != 0)
                return false;
    return true;
}

void snapshot(const Inventory &src, Chest_MM7 *dst) {
    //snapshot(src._items, &dst->igChestItems);
    //snapshot(src._storage, &dst->pInventoryIndices, tags::cast<int, int16_t>);
}

void reconstruct(const Chest_MM7 &src, Inventory *dst) {
    //reconstruct(src.igChestItems, &dst->_items);
    //reconstruct(src.pInventoryIndices, &dst->_storage, tags::cast<int16_t, int>);
}
