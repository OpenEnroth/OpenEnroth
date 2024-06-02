#pragma once

#include <cstdint>
#include <cassert>
#include <utility>


enum class ObjectType {
    OBJECT_None = 0x0,
    OBJECT_Door = 0x1,          // Pid id is index in pIndoor->pDoors.
    OBJECT_Item = 0x2,          // Pid id is index in pSpriteObjects array. Note that not all sprite objects are items.
    OBJECT_Actor = 0x3,         // Pid id is index in pActors array.
    OBJECT_Character = 0x4,     // Pid id is character index in [0..3].
    OBJECT_Decoration = 0x5,    // Pid id is index in pLevelDecorations array.
    OBJECT_Face = 0x6,          // Pid id is ((model_id << 6) + face_id) outdoors, face_id indoors.
    OBJECT_Light = 0x7,
};
using enum ObjectType;

class Pid {
 public:
    enum {
        ID_MAX = 0xFFFF >> 3,
        ODM_FACE_ID_MAX = 0x3F,
        ODM_MODEL_ID_MAX = 0x7F
    };

    constexpr Pid() = default;

    constexpr Pid(ObjectType objectType, int id) {
        assert(id >= 0 && id <= ID_MAX);
        _value = (id << 3) + std::to_underlying(objectType);
    }

    static constexpr Pid door(int id) {
        return Pid(OBJECT_Door, id);
    }

    static constexpr Pid item(int id) {
        return Pid(OBJECT_Item, id);
    }

    static constexpr Pid actor(int id) {
        return Pid(OBJECT_Actor, id);
    }

    static constexpr Pid character(int id) {
        assert(id >= 0 && id <= 5); // TODO(captainurist): Should be <= 3, but 4-5 are used for hirelings in UIMSG_OnCastTownPortal.
        return Pid(OBJECT_Character, id);
    }

    static constexpr Pid decoration(int id) {
        return Pid(OBJECT_Decoration, id);
    }

    static constexpr Pid odmFace(int modelId, int faceId) {
        assert(modelId >= 0 && modelId <= ODM_MODEL_ID_MAX);
        assert(faceId >= 0 && faceId <= ODM_FACE_ID_MAX);
        return Pid(OBJECT_Face, (modelId << 6) + faceId);
    }

    static constexpr Pid blvFace(int id) {
        return Pid(OBJECT_Face, id);
    }

    /**
     * @return                          A dummy `Pid` that's valid, but doesn't represent any object.
     */
    static constexpr Pid dummy() {
        return Pid(OBJECT_Light, ID_MAX);
    }

    static constexpr Pid fromPacked(uint16_t value) {
        Pid result;
        result._value = value;
        return result;
    }

    [[nodiscard]] constexpr ObjectType type() const {
        return static_cast<ObjectType>(_value & 0x7);
    }

    [[nodiscard]] constexpr int id() const {
        return _value >> 3;
    }

    [[nodiscard]] constexpr uint16_t packed() const {
        return _value;
    }

    friend constexpr bool operator==(const Pid &l, const Pid &r) = default;

    explicit constexpr operator bool() const {
        return _value != 0;
    }

    constexpr bool operator!() const {
        return _value == 0;
    }

 private:
    uint16_t _value = 0;
};
