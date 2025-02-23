#pragma once

#include <array>
#include <ranges>
#include <optional>

#include "Library/Geometry/Size.h"
#include "Library/Geometry/Point.h"
#include "Library/Geometry/Rect.h"

#include "ItemEnums.h"
#include "Item.h"

struct Chest_MM7;

// SO:
// - ChestInventory : private Inventory, expose only relevant methods.
// - snapshot / reconstruct should use public methods. So, not friends. This way we'll have 100% consistency.
// - Add unit tests === actually easy, it's a GAME TEST. Link to game tests & you'll have everything you need. Just don't use game. and test.
// - And write better docs.

enum class InventoryLocation {
    /** Item is effectively hidden. Applies only to chest inventory. MM7 had chests that were filled to the brim with
     * stuff, so that there was simply not enough chest space to place all the items, and some items ended up hidden.
     * See `chest_try_place_items` config option. */
    INVENTORY_BURIED = 0,

    /** Item is stored in the inventory grid. */
    INVENTORY_STORED = 1,

    /** Item is worn by a character. Applies only to character inventory. */
    INVENTORY_EQUIPPED = 2,
};
using enum InventoryLocation;

class Inventory {
 public:
    static constexpr std::size_t MAX_ITEMS = 140;

    explicit Inventory(Sizei storageSize = Sizei(1, 1), int capacity = MAX_ITEMS);

    [[nodiscard]] Sizei storageSize() const {
        return _storageSize;
    }

    [[nodiscard]] int size() const {
        return _size;
    }

    [[nodiscard]] int capacity() const {
        return _capacity;
    }

    [[nodiscard]] Recti storageRect() const {
        return Recti(Pointi(), _storageSize);
    }

    [[nodiscard]] auto items() {
        return _items
            | std::views::filter([](auto &&item) { return item.itemId != ITEM_NULL; })
            | std::views::transform([](auto &&item) { return std::addressof(item); });
    }

    [[nodiscard]] auto items() const {
        // TODO(captainurist): copypasta. Use deduced this.
        return _items
            | std::views::filter([](auto &&item) { return item.itemId != ITEM_NULL; })
            | std::views::transform([](auto &&item) { return std::addressof(item); });
    }

    [[nodiscard]] InventoryLocation location(const Item *item) const;

    [[nodiscard]] Recti geometry(const Item *item) const;

    [[nodiscard]] ItemSlot slot(const Item *item) const;

    Item *storedItem(Pointi position);

    const Item *storedItem(Pointi position) const {
        return const_cast<Inventory *>(this)->storedItem(position);
    }

    Item *equippedItem(ItemSlot slot);

    const Item *equippedItem(ItemSlot slot) const {
        return const_cast<Inventory *>(this)->equippedItem(slot);
    }

    [[nodiscard]] bool canStoreItem(Pointi position, Sizei size) const;
    Item *storeItem(Pointi position, const Item &item);

    [[nodiscard]] bool canEquipItem(ItemSlot slot) const;
    Item *equipItem(ItemSlot slot, const Item &item);

    [[nodiscard]] bool canBuryItem() const;
    Item *buryItem(const Item &item);

    Item takeItem(Item *item);

    [[nodiscard]] std::optional<Pointi> findStorage(Sizei size) const;

 private:
    int findFreeIndex() const;
    bool isStorageFree(Pointi position, Sizei size) const;

 private:
    /** Inventory storage area size in cells. */
    Sizei _storageSize;

    /** Number of items in this inventory. */
    int _size = 0;

    /** Max number of items that can be stuffed into this inventory. */
    int _capacity = 0;

    /** All items in inventory. `ITEM_NULL` means the slot is empty. */
    std::array<Item, MAX_ITEMS> _items;

    /** Is an item hidden? MM7 had chests that were filled to the brim with stuff, so that there was simply not enough
     * chest space to place all the items, and some items ended up effectively hidden. See `chest_try_place_items`
     * config option. */
    std::array<InventoryLocation, MAX_ITEMS> _locations = {{}};

    /** Item positions. Only valid for items stored in grid. */
    std::array<Pointi, MAX_ITEMS> _positions;

    /** Item slots. Only valid for equipped items. */
    std::array<ItemSlot, MAX_ITEMS> _slots = {{}};

    /** Backpack matrix. 0 means empty cell. Positive number is an index into `_items` plus one. Negative number is
     * an index into the main slot in `_storage` minus one. */
    std::array<int, MAX_ITEMS> _storage = {{}};

    /** Equipment array. Positive number is an index into `_items` plus one. Zero means empty. */
    IndexedArray<int, ITEM_SLOT_FIRST_VALID, ITEM_SLOT_LAST_VALID> _equipment = {{}};
};

void snapshot(const Inventory &src, Chest_MM7 *dst);
void reconstruct(const Chest_MM7 &src, Inventory *dst);
