#pragma once

#include <string>
#include <string_view>
#include <ranges>
#include <vector>

#include "Engine/Graphics/Outdoor.h"
#include "Engine/Objects/CharacterEnums.h"

#include "Library/Geometry/Point.h"
#include "Library/Geometry/Rect.h"
#include "Library/Logger/DistLogSink.h"
#include "Library/Logger/LogSink.h"

#include "Utility/Ptr.h"

class GraphicsImage;

/**
 * @param model                         Outdoor BSP model.
 * @return                              View of the model's faces in `pOutdoor->faces`.
 */
inline auto bmodelFaces(const BSPModel &model) {
    return model.faces | std::views::transform([](int faceId) -> const BLVFace & { return pOutdoor->faces[faceId]; });
}

struct CharacterPreset {
    Class classType;
    Race race;
};

/**
 * Sets up a battle test on Emerald Island:
 * - Moves the party in front of the bridge;
 * - Applies wizard's eye;
 * - Configures one character per entry in `presets` (class & race), killing off the rest;
 * - Chonks each configured character (level / endurance / bodybuilding) so they don't die in combat;
 * - Zeroes out luck on each configured character so that luck rolls don't randomly bump damage dealt to them.
 */
void prepareForBattleTest(const std::vector<CharacterPreset> &presets = {{CLASS_KNIGHT, RACE_GOBLIN}});

// Captures every WARNING+ message written to it, so a `LogHygieneScope` can inspect what
// was logged during its lifetime. Only used through `LogHygieneScope` below.
class CapturingLogSink : public LogSink {
 public:
    void write(const LogCategory &category, LogLevel level, std::string_view message) override;

    const std::vector<std::string> &messages() const {
        return _messages;
    }

 private:
    std::vector<std::string> _messages;
};

// Taps the logger for the scope's lifetime and captures every WARNING+ message, so a test can assert
// none of them signal data loss / missing assets / unmapped enums. Wraps the installed sink
// in a `DistLogSink` and restores it on scope exit.
class LogHygieneScope {
 public:
    LogHygieneScope();
    ~LogHygieneScope();

    const std::vector<std::string> &messages() const {
        return _capture.messages();
    }

    // Every captured WARNING+ line that both (a) matches one of the forbidden data-loss/missing-
    // asset/unmapped-enum substrings and (b) isn't covered by `isLogHygieneAllowlisted` below.
    std::vector<std::string> violations() const;

 private:
    LogSink *_originalSink;
    CapturingLogSink _capture;
    DistLogSink _dist;
};

// Allowlist: known gaps that are expected to still warn. Remove an entry once its gap is fixed.
bool isLogHygieneAllowlisted(std::string_view message);

// Picks an opaque pixel of a doll equipment layer's texture inside `clickableRegion`. `HitMap::query` applies
// the same alpha test, so a click there is a hit. Some doll layers overflow the doll box, so not every opaque
// pixel is clickable. Returns `{-1, -1}` if there is none.
Pointi findClickableOpaquePixel(Ptr<GraphicsImage> texture, Pointi dstTopLeft, const Recti &clickableRegion);
