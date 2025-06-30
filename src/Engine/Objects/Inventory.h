#pragma once

#include <cassert>
#include <array>
#include <ranges>
#include <optional>
#include <span>
#include <type_traits>

#include "Inventory.h"
#include "Library/Geometry/Size.h"
#include "Library/Geometry/Point.h"
#include "Library/Geometry/Rect.h"

#include "Library/Binary/BinaryTags.h"

#include "ItemEnums.h"
#include "Item.h"

struct Chest_MM7;
struct Character_MM7;
class ChestInventory;
class CharacterInventory;
class Inventory;

/**
 * Logical location of an `Item` inside an `Inventory`.
 */
enum class InventoryZone {
    /** This is for invalid inventory entries. */
    INVENTORY_ZONE_INVALID = 0,

    /** Item is effectively hidden. Applies only to chest inventory. MM7 had chests that were filled to the brim with
     * stuff, so that there was simply not enough chest space to place all the items, and some items ended up hidden.
     * See `chest_try_place_items` config option. */
    INVENTORY_ZONE_STASH = 1,

    /** Item is stored in the inventory grid. */
    INVENTORY_ZONE_GRID = 2,

    /** Item is worn by a character. Applies only to character inventory. */
    INVENTORY_ZONE_EQUIPMENT = 3,
};
using enum InventoryZone;


/**
 * Lightweight, pointer‑like descriptor giving `const` access to an item owned by an `Inventory`.
 *
 * `InventoryConstEntry` can be in an invalid state, `index()` will return `-1` in this case, all other methods will
 * assert. Check for validity with `operator bool`.
 */
class InventoryConstEntry {
 public:
    InventoryConstEntry() = default;

    [[nodiscard]] inline explicit operator bool() const;
    [[nodiscard]] inline const Item *operator->() const;
    [[nodiscard]] inline const Item &operator*() const;
    [[nodiscard]] inline const Item *get() const;
    [[nodiscard]] inline InventoryZone zone() const;
    [[nodiscard]] inline Recti geometry() const;
    [[nodiscard]] inline ItemSlot slot() const;
    [[nodiscard]] inline int index() const;

    [[nodiscard]] bool operator!() const {
        return !static_cast<bool>(*this);
    }

    InventoryConstEntry &operator=(std::nullptr_t) {
        *this = InventoryConstEntry();
        return *this;
    }

 protected:
    friend class Inventory;

    inline InventoryConstEntry(const Inventory *inventory, int index);

 private:
    const Inventory *_inventory = nullptr;
    int _index = -1;
};


/**
 * Mutable counterpart to `InventoryConstEntry`, allowing modification of the referenced `Item`.
 */
class InventoryEntry : public InventoryConstEntry {
 public:
    InventoryEntry() = default;

    [[nodiscard]] Item *operator->() const {
        return const_cast<Item *>(InventoryConstEntry::operator->());
    }

    [[nodiscard]] Item &operator*() const {
        return const_cast<Item &>(InventoryConstEntry::operator*());
    }

    [[nodiscard]] Item *get() const {
        return const_cast<Item *>(InventoryConstEntry::get());
    }

    InventoryEntry &operator=(std::nullptr_t) {
        *this = InventoryEntry();
        return *this;
    }

 protected:
    friend class Inventory;

    InventoryEntry(Inventory *inventory, int index) : InventoryConstEntry(inventory, index) {}
};


/**
 * Core container that stores items for both chests and characters.
 *
 * `Inventory` stores items inside an array, so all items have indices. These indices are exposed through
 * `InventoryConstEntry::index` and `Inventory::entry(int)`. In practice there shouldn't be a lot of use cases where
 * direct access to item indices is needed.
 *
 * @see ChestInventory
 * @see CharacterInventory
 */
class Inventory {
 public:
    static constexpr std::size_t MAX_ITEMS = 140;

    /**
     * @param gridSize                  Size of inventory's grid, WxH must be less or equal to `MAX_ITEMS`.
     * @param capacity                  Inventory capacity, must be less or equal to `MAX_ITEMS`.
     */
    explicit Inventory(Sizei gridSize, int capacity);

    /**
     * Constructs a 1x1 inventory that can hold `MAX_ITEMS` items.
     */
    Inventory() : Inventory(Sizei(1, 1), MAX_ITEMS) {}

    /**
     * @return                          How many items are currently held in this inventory object.
     */
    [[nodiscard]] int size() const {
        return _size;
    }

    /**
     * @return                          How many items can this inventory object hold.
     */
    [[nodiscard]] int capacity() const {
        return _capacity;
    }

    /**
     * @return                          Size of this inventory's grid, in cells.
     */
    [[nodiscard]] Sizei gridSize() const {
        return _gridSize;
    }

    /**
     * @return                          Rect for this inventory's grid, in cells. Mainly useful for checks like
     *                                  `inventory.gridRect().contains(pos)`.
     */
    [[nodiscard]] Recti gridRect() const {
        return Recti(Pointi(), _gridSize);
    }

    /**
     * @param self                      `*this`.
     * @return                          A range of `InventoryEntry` or `InventoryConstEntry` objects for items in this
     *                                  inventory. Returned entries are never invalid.
     */
    [[nodiscard]] auto entries(this auto &&self) {
        return std::views::iota(0, self._capacity)
            | std::views::filter([&self](int i) { return self._records[i].item.itemId != ITEM_NULL; })
            | std::views::transform([&self](int i) { return std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, InventoryConstEntry, InventoryEntry>(&self, i); });
    }

    /**
     * @param self                      `*this`.
     * @return                          A range of `Item` objects for items in this inventory. Returned items are never
     *                                  `ITEM_NULL`.
     */
    [[nodiscard]] auto items(this auto &&self) {
        return self.availableRecords()
            | std::views::filter([](auto &&record) { return record.item.itemId != ITEM_NULL; })
            | std::views::transform(&InventoryRecord::item);
    }

    /**
     * @param self                      `*this`.
     * @param itemId                    Item id to filter for.
     * @return                          A range of `Item` objects for items in this inventory with the given `itemId`.
     */
    [[nodiscard]] auto items(this auto &&self, ItemId itemId) {
        return self.availableRecords()
            | std::views::filter([itemId](auto &&record) { return record.item.itemId == itemId; })
            | std::views::transform(&InventoryRecord::item);
    }

    /**
     * @param position                  Grid position to look up an item.
     * @return                          Inventory entry at provided grid position, or an invalid entry if `position` is
     *                                  out of bounds or if the grid at `position` is empty.
     */
    [[nodiscard]] InventoryEntry entry(Pointi position);
    [[nodiscard]] InventoryConstEntry entry(Pointi position) const {
        return const_cast<Inventory *>(this)->entry(position);
    }

    /**
     * @param slot                      Equipment slot to look up an item.
     * @return                          Inventory entry for an item equipped in `slot`, or an invalid entry if that slot
     *                                  is empty.
     */
    [[nodiscard]] InventoryEntry entry(ItemSlot slot);
    [[nodiscard]] InventoryConstEntry entry(ItemSlot slot) const {
        return const_cast<Inventory *>(this)->entry(slot);
    }

    /**
     * @param index                     Entry index to look up an item.
     * @return                          Inventory entry at `index`, or an invalid entry if `index` is out of bounds or
     *                                  if there is no item at `index`.
     */
    [[nodiscard]] InventoryEntry entry(int index);
    [[nodiscard]] InventoryConstEntry entry(int index) const {
        return const_cast<Inventory *>(this)->entry(index);
    }

    [[nodiscard]] bool canAdd(Pointi position, Sizei size) const;
    [[nodiscard]] bool canAdd(Pointi position, const Item &item) const {
        return canAdd(position, item.inventorySize());
    }
    [[nodiscard]] bool canAdd(Sizei size) const;
    [[nodiscard]] bool canAdd(const Item &item) const {
        return canAdd(item.inventorySize());
    }
    InventoryEntry add(Pointi position, const Item &item);
    InventoryEntry add(const Item &item);
    InventoryEntry tryAdd(Pointi position, const Item &item);
    InventoryEntry tryAdd(const Item &item);

    [[nodiscard]] bool canEquip(ItemSlot slot) const;
    InventoryEntry equip(ItemSlot slot, const Item &item);
    InventoryEntry tryEquip(ItemSlot slot, const Item &item);

    [[nodiscard]] bool canStash() const;
    InventoryEntry stash(const Item &item);
    InventoryEntry tryStash(const Item &item);

    Item take(InventoryEntry entry);

    [[nodiscard]] std::optional<Pointi> findSpace(Sizei size) const;
    [[nodiscard]] std::optional<Pointi> findSpace(const Item &item) const {
        return findSpace(item.inventorySize());
    }

    [[nodiscard]] InventoryEntry find(ItemId itemId);
    [[nodiscard]] InventoryConstEntry find(ItemId itemId) const {
        return const_cast<Inventory *>(this)->find(itemId);
    }

    void clear();

    friend void snapshot(const ChestInventory &src, Chest_MM7 *dst);
    friend void reconstruct(const Chest_MM7 &src, ChestInventory *dst, ContextTag<int> chestId);
    friend void snapshot(const CharacterInventory &src, Character_MM7 *dst);
    friend void reconstruct(const Character_MM7 &src, CharacterInventory *dst, ContextTag<int> characterIndex);

 private:
    friend class InventoryConstEntry;

    struct InventoryRecord {
        Item item;
        InventoryZone zone = INVENTORY_ZONE_INVALID;
        Pointi position;
        ItemSlot slot = ITEM_SLOT_INVALID;
    };

    [[nodiscard]] int findFreeIndex() const;
    [[nodiscard]] bool isGridFree(Pointi position, Sizei size) const;
    InventoryEntry addAt(Pointi position, const Item &item, int index);
    InventoryEntry equipAt(ItemSlot slot, const Item &item, int index);
    InventoryEntry stashAt(const Item &item, int index);
    void checkInvariants() const;

    [[nodiscard]] auto availableRecords(this auto &&self) {
        return std::span(self._records.data(), self._records.data() + self._capacity);
    }

 private:
    /** Inventory storage area size in cells. */
    Sizei _gridSize;

    /** Number of items in this inventory. */
    int _size = 0;

    /** Max number of items that can be stuffed into this inventory. */
    int _capacity = 0;

    /** All items in inventory. `ITEM_NULL` means the slot is empty. */
    std::array<InventoryRecord, MAX_ITEMS> _records;

    /** Backpack grid. 0 means empty cell. Positive number is an index into `_records` plus one. Negative number is
     * an index into the main slot in `_grid` minus one. */
    std::array<int, MAX_ITEMS> _grid = {{}};

    /** Equipment array. Positive number is an index into `_records` plus one. Zero means empty. */
    IndexedArray<int, ITEM_SLOT_FIRST_VALID, ITEM_SLOT_LAST_VALID> _equipment = {{}};
};


/**
 * Thin facade that exposes only the chest‑appropriate subset of `Inventory` interface.
 */
class ChestInventory : private Inventory {
 public:
    // Using Inventory::MAX_ITEMS here because chests can be filled to the brim with stuff beyond the obvious
    // capacity=WxH limit, and I believe this wasn't enforced in any way by the engine.
    explicit ChestInventory(Sizei size): Inventory(size, MAX_ITEMS) {}
    ChestInventory(): ChestInventory(Sizei(1, 1)) {}

    using Inventory::size;
    using Inventory::capacity;
    using Inventory::gridSize;
    using Inventory::gridRect;
    using Inventory::entries;
    using Inventory::items;
    using Inventory::entry;
    using Inventory::canAdd;
    using Inventory::add;
    using Inventory::tryAdd;
    using Inventory::canStash;
    using Inventory::stash;
    using Inventory::tryStash;
    using Inventory::take;
    using Inventory::findSpace;
    using Inventory::find;
    using Inventory::clear;

    friend void snapshot(const ChestInventory &src, Chest_MM7 *dst);
    friend void reconstruct(const Chest_MM7 &src, ChestInventory *dst, ContextTag<int> chestId);

 private:
    friend class Inventory;
};


/**
 * Thin facade that exposes only the character‑appropriate subset of `Inventory` interface.
 */
class CharacterInventory : private Inventory {
 public:
    CharacterInventory(): Inventory(Sizei(14, 9), 14 * 9) {}

    using Inventory::size;
    using Inventory::capacity;
    using Inventory::gridSize;
    using Inventory::gridRect;
    using Inventory::entries;
    using Inventory::items;
    using Inventory::entry;
    using Inventory::canAdd;
    using Inventory::add;
    using Inventory::tryAdd;
    using Inventory::canEquip;
    using Inventory::equip;
    using Inventory::tryEquip;
    using Inventory::take;
    using Inventory::findSpace;
    using Inventory::find;
    using Inventory::clear;

    friend void snapshot(const CharacterInventory &src, Character_MM7 *dst);
    friend void reconstruct(const Character_MM7 &src, CharacterInventory *dst, ContextTag<int> characterIndex);

 private:
    friend class Inventory;
};


//
// InventoryConstEntry implementation follows.
//

InventoryConstEntry::InventoryConstEntry(const Inventory *inventory, int index) : _inventory(inventory), _index(index) {
    assert(_inventory);
    assert(_index >= 0 && _index < _inventory->_capacity);
}

InventoryConstEntry::operator bool() const {
    return _inventory && _inventory->_records[_index].item.itemId != ITEM_NULL;
}

const Item *InventoryConstEntry::operator->() const {
    assert(!!*this);
    return get();
}

const Item &InventoryConstEntry::operator*() const {
    assert(!!*this);
    return *get();
}

inline const Item *InventoryConstEntry::get() const {
    return *this ? &_inventory->_records[_index].item : nullptr;
}

InventoryZone InventoryConstEntry::zone() const {
    return *this ? _inventory->_records[_index].zone : INVENTORY_ZONE_INVALID;
}

Recti InventoryConstEntry::geometry() const {
    if (!*this)
        return {};
    const auto &data = _inventory->_records[_index];
    return data.zone == INVENTORY_ZONE_GRID ? Recti(data.position, data.item.inventorySize()) : Recti();
}

ItemSlot InventoryConstEntry::slot() const {
    return *this ? _inventory->_records[_index].slot : ITEM_SLOT_INVALID;
}

int InventoryConstEntry::index() const {
    return _index;
}
