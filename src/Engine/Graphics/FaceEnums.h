#pragma once

#include <cstdint>

#include "Utility/Flags.h"

enum class FaceAttribute : uint32_t {
    FACE_IsPortal          = 0x00000001,
    FACE_IsSecret          = 0x00000002,
    FACE_FlowDown          = 0x00000004,
    FACE_TexAlignDown      = 0x00000008,
    FACE_IsFluid           = 0x00000010,
    FACE_FlowUp            = 0x00000020,
    FACE_FlowLeft          = 0x00000040,
    FACE_SeenByParty       = 0x00000080,
    FACE_XY_PLANE          = 0x00000100,
    FACE_XZ_PLANE          = 0x00000200,
    FACE_YZ_PLANE          = 0x00000400,
    FACE_FlowRight         = 0x00000800,
    FACE_TexAlignLeft      = 0x00001000,
    FACE_IsInvisible       = 0x00002000,
    FACE_TEXTURE_FRAME     = 0x00004000,  // Texture ID is a frameset from TextureFrameTable, otherwise BitmapID
    FACE_TexAlignRight     = 0x00008000,
    FACE_OUTLINED          = 0x00010000,  // outline face debug
    FACE_TexAlignBottom    = 0x00020000,
    FACE_TexMoveByDoor     = 0x00040000,
    FACE_UNKOWN_10         = 0x00080000,  // MMExt: TriggerByTouch, doesn't work anymore
    FACE_HAS_EVENT         = 0x00100000,  // MMExt: IsEventJustHint, [MM7+] // TODO(captainurist): rename, this is not HAS_EVENT
    FACE_INDOOR_CARPET     = 0x00200000,  // MMExt: AlternativeSound
    FACE_INDOOR_SKY        = 0x00400000,  // MMExt: outdoor in software mode: horizontal flow
    FACE_FlipNormalU       = 0x00800000,
    FACE_FlipNormalV       = 0x01000000,
    FACE_CLICKABLE         = 0x02000000,  // Event can be triggered by clicking on the facet.
    FACE_PRESSURE_PLATE    = 0x04000000,  // Event can be triggered by stepping on the facet.
    FACE_INDICATE          = 0x06000000,  // face has status bar string on hover
    FACE_TriggerByMonster  = 0x08000000,
    FACE_TriggerByObject   = 0x10000000,
    FACE_ETHEREAL          = 0x20000000,  // Untouchable. You can pass through it.
    // MMExt: great for vertical facets of stairs.
    // [MM7+] Shouldn't be used for sloped floor, like it's used in MM6.
    FACE_IsLava            = 0x40000000,
    FACE_IsPicked          = 0x80000000,  // Was used internally in MM7 to mark faces for setting FACE_OUTLINED on them,
                                          // we just set FACE_OUTLINED directly.
    // TODO: MMExt: HasData, are we talking about BLVFaceExtra?
};
using enum FaceAttribute;
MM_DECLARE_FLAGS(FaceAttributes, FaceAttribute)
MM_DECLARE_OPERATORS_FOR_FLAGS(FaceAttributes)

// door attr
enum class DoorAttribute : uint32_t {
    DOOR_TRIGGERED         = 0x00000001,
    DOOR_SETTING_UP        = 0x00000002, // unused
    DOOR_NOSOUND           = 0x00000004,
};
using enum DoorAttribute;
MM_DECLARE_FLAGS(DoorAttributes, DoorAttribute)
MM_DECLARE_OPERATORS_FOR_FLAGS(DoorAttributes)

// TODO(captainurist): most closed doors are in DOOR_OPEN, and most open doors are in DOOR_CLOSED. Rename states?
enum class DoorState : uint16_t {
    DOOR_OPEN = 0, // Initial state, door mesh is at a position where BLVDoor::p[XYZ]Offsets point.
    DOOR_CLOSING = 1, // Going into alternative state.
    DOOR_CLOSED = 2, // Alternative state, door mesh is at BLVDoor::p[XYZ]Offsets + BLVDoor::vDirection * BLVDoor::uMoveLength.
    DOOR_OPENING = 3, // Going into initial state.
};
using enum DoorState;

enum class DoorAction {
    DOOR_ACTION_OPEN = 0, // If closed or closing, transitions to opening.
    DOOR_ACTION_CLOSE = 1, // If open or opening, transitions to closing.
    DOOR_ACTION_TRIGGER = 2, // Only works on fully open / closed doors. Closes / opens the door.
};
using enum DoorAction;

enum class PolygonType : uint8_t {
    POLYGON_Invalid = 0x0,
    POLYGON_VerticalWall = 0x1,
    POLYGON_unk = 0x2,
    POLYGON_Floor = 0x3,
    POLYGON_InBetweenFloorAndWall = 0x4,
    POLYGON_Ceiling = 0x5,
    POLYGON_InBetweenCeilingAndWall = 0x6,
};
using enum PolygonType;
