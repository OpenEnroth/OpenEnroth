#pragma once

#include <array>
#include <ranges>
#include <optional>

#include "Library/Geometry/Size.h"
#include "Library/Geometry/Point.h"
#include "Library/Geometry/Rect.h"

#include "Library/Binary/BinaryTags.h"

#include "ItemEnums.h"
#include "Item.h"

struct Chest_MM7;
class ChestInventory;

enum class InventoryZone {
    /** Item is effectively hidden. Applies only to chest inventory. MM7 had chests that were filled to the brim with
     * stuff, so that there was simply not enough chest space to place all the items, and some items ended up hidden.
     * See `chest_try_place_items` config option. */
    INVENTORY_ZONE_HIDDEN = 0,

    /** Item is stored in the inventory grid. */
    INVENTORY_ZONE_GRID = 1,

    /** Item is worn by a character. Applies only to character inventory. */
    INVENTORY_ZONE_EQUIPMENT = 2,
};
using enum InventoryZone;

struct InventoryEntry {
 public:
    InventoryEntry() = default;

    /**
     * @returns                         Item in this entry. If you got this entry from one of `Inventory` methods,
     *                                  it will never be `ITEM_NULL`.
     */
    [[nodiscard]] const Item &item() const {
        return _item;
    }

    [[nodiscard]] Item &item() {
        return _item;
    }

    /**
     * @returns                         Where is the item located.
     */
    [[nodiscard]] InventoryZone zone() const {
        return _zone;
    }

    /**
     * @returns                         Only for items in grid - item geometry in inventory cells.
     */
    [[nodiscard]] Recti geometry() const {
        return _zone == INVENTORY_ZONE_GRID ? Recti(_position, _item.inventorySize()) : Recti();
    }

    /**
     * @returns                         Only for equipped items - item slot.
     */
    [[nodiscard]] ItemSlot slot() const {
        return _slot;
    }

 private:
    friend class Inventory;

 private:
    Item _item;
    InventoryZone _zone = INVENTORY_ZONE_HIDDEN;
    Pointi _position;
    ItemSlot _slot = ITEM_SLOT_INVALID;
};

class Inventory {
 public:
    static constexpr std::size_t MAX_ITEMS = 140;

    explicit Inventory(Sizei storageSize = Sizei(1, 1), int capacity = MAX_ITEMS);

    /**
     * @return                          How many items are currently held in this inventory object.
     */
    [[nodiscard]] int size() const {
        return _size;
    }

    /**
     * @return                          How many items can this inventory object hold, it's always `WxH`.
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
     * @return                          A range of `InventoryEntry *` pointers for items in this inventory. Returned
     *                                  pointers are never null, and items are never `ITEM_NULL`.
     */
    [[nodiscard]] auto entries(this auto &&self) {
        return self._entries
            | std::views::filter([](auto &&entry) { return entry._item.itemId != ITEM_NULL; })
            | std::views::transform([](auto &&entry) { return std::addressof(entry); });
    }

    /**
     * @return                          A range of `Item` objects for items in this inventory. Returned items are never
     *                                  `ITEM_NULL`.
     */
    [[nodiscard]] auto items(this auto &&self) {
        return self._entries
            | std::views::filter([](auto &&entry) { return entry._item.itemId != ITEM_NULL; })
            | std::views::transform([](auto &&entry) { return entry._item; });
    }

    /**
     * @param position                  Grid position to look up an item.
     * @return                          Item at provided grip position, or `nullptr` if `position` is out of bounds or
     *                                  if the grid at `position` is empty.
     */
    [[nodiscard]] InventoryEntry *gridItem(Pointi position);
    [[nodiscard]] const InventoryEntry *gridItem(Pointi position) const {
        return const_cast<Inventory *>(this)->gridItem(position);
    }

    /**
     * @param slot                      Equipment slot to look up an item.
     * @return                          Item equipped in `slot`, or `nullptr` if nothing is equipped in `slot`.
     */
    [[nodiscard]] InventoryEntry *equippedItem(ItemSlot slot);
    [[nodiscard]] const InventoryEntry *equippedItem(ItemSlot slot) const {
        return const_cast<Inventory *>(this)->equippedItem(slot);
    }

    [[nodiscard]] bool canAddGridItem(Pointi position, Sizei size) const;
    InventoryEntry *addGridItem(Pointi position, const Item &item);

    [[nodiscard]] bool canAddEquippedItem(ItemSlot slot) const;
    InventoryEntry *addEquippedItem(ItemSlot slot, const Item &item);

    [[nodiscard]] bool canAddHiddenItem() const;
    InventoryEntry *addHiddenItem(const Item &item);

    Item takeItem(InventoryEntry *entry);

    [[nodiscard]] std::optional<Pointi> findGridSpace(Sizei size) const;
    [[nodiscard]] std::optional<Pointi> findGridSpace(const Item &item) const {
        return findGridSpace(item.inventorySize());
    }

    [[nodiscard]] InventoryEntry *findEntry(ItemId itemId);
    [[nodiscard]] const InventoryEntry *findEntry(ItemId itemId) const {
        return const_cast<Inventory *>(this)->findEntry(itemId);
    }

    friend void snapshot(const ChestInventory &src, Chest_MM7 *dst);
    friend void reconstruct(const Chest_MM7 &src, ChestInventory *dst, ContextTag<int> chestId);

 private:
    int findFreeIndex() const;
    bool isGridFree(Pointi position, Sizei size) const;
    InventoryEntry *addGridItemAtIndex(Pointi position, const Item &item, int index);
    InventoryEntry *addHiddenItemAtIndex(const Item &item, int index);
    void checkInvariants() const;

 private:
    /** Inventory storage area size in cells. */
    Sizei _gridSize;

    /** Number of items in this inventory. */
    int _size = 0;

    /** Max number of items that can be stuffed into this inventory. */
    int _capacity = 0;

    /** All items in inventory. `ITEM_NULL` means the slot is empty. */
    std::array<InventoryEntry, MAX_ITEMS> _entries;

    /** Backpack grid. 0 means empty cell. Positive number is an index into `_entries` plus one. Negative number is
     * an index into the main slot in `_grid` minus one. */
    std::array<int, MAX_ITEMS> _grid = {{}};

    /** Equipment array. Positive number is an index into `_entries` plus one. Zero means empty. */
    IndexedArray<int, ITEM_SLOT_FIRST_VALID, ITEM_SLOT_LAST_VALID> _equipment = {{}};
};

class ChestInventory : private Inventory {
 public:
    using Inventory::Inventory;
    using Inventory::size;
    using Inventory::capacity;
    using Inventory::gridSize;
    using Inventory::gridRect;
    using Inventory::entries;
    using Inventory::items;
    using Inventory::gridItem;
    using Inventory::canAddGridItem;
    using Inventory::addGridItem;
    using Inventory::canAddHiddenItem;
    using Inventory::addHiddenItem;
    using Inventory::takeItem;
    using Inventory::findGridSpace;
    using Inventory::findEntry;

    friend void snapshot(const ChestInventory &src, Chest_MM7 *dst);
    friend void reconstruct(const Chest_MM7 &src, ChestInventory *dst, ContextTag<int> chestId);

 private:
    friend class Inventory;
};
