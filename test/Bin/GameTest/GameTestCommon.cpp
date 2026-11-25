#include "GameTestCommon.h"

#include <fmt/format.h>

#include <cassert>
#include <string>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Party.h"

#include "Library/Logger/Logger.h"

static int faceForRace(Race race) {
    switch (race) {
    default: assert(false); [[fallthrough]];
    case RACE_HUMAN:  return 0;
    case RACE_ELF:    return 8;
    case RACE_DWARF:  return 12;
    case RACE_GOBLIN: return 17; // Default char0 face - keep voice & portrait stable for the no-arg case.
    }
}

void prepareForBattleTest(const std::vector<CharacterPreset> &presets) {
    assert(presets.size() >= 1 && presets.size() <= pParty->pCharacters.size());
    assert(engine->_currentLoadedMapId == MAP_EMERALD_ISLAND);

    // Move party in front of the bridge.
    pParty->pos = Vec3f(12552, 2000, 1);

    // Wizard's eye is handy for debugging.
    Time tomorrow = pParty->GetPlayingTime() + Duration::fromDays(1);
    pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Apply(tomorrow, MASTERY_GRANDMASTER, 30, 0, 0);

    // Apply class & race to the configured chars; portrait & voice follow the race.
    for (int i = 0; i < presets.size(); ++i) {
        Character &c = pParty->pCharacters[i];
        c.ChangeClass(presets[i].classType);
        c.uCurrentFace = faceForRace(presets[i].race);
        c.uPrevFace = c.uCurrentFace;
        c.uVoiceID = c.uCurrentFace;
        c.uPrevVoiceID = c.uCurrentFace;
        c.uSex = c.GetSexByVoice();
    }

    // Kill off characters past the configured set.
    for (int i = presets.size(); i < pParty->pCharacters.size(); ++i)
        pParty->pCharacters[i].SetCondDeadWithBlockCheck(false);

    // Chonk each configured character so they survive incoming attacks.
    for (int i = 0; i < presets.size(); ++i) {
        Character &c = pParty->pCharacters[i];
        c.sLevelModifier = 5000;
        c._stats[ATTRIBUTE_ENDURANCE] = 500;
        c.setSkillValue(SKILL_BODYBUILDING, CombinedSkillValue(63, MASTERY_GRANDMASTER));
        c.health = c.GetMaxHealth();
        c._stats[ATTRIBUTE_LUCK] = 0; // We don't want luck rolls that decrease damage dealt.
    }
}

void CapturingLogSink::write(const LogCategory &category, LogLevel level, std::string_view message) {
    if (level >= LOG_WARNING)
        _messages.emplace_back(message);
}

LogHygieneScope::LogHygieneScope() : _originalSink(detail::logger->sink()) {
    _dist.addLogSink(_originalSink);
    _dist.addLogSink(&_capture);
    detail::logger->setSink(&_dist);
}

LogHygieneScope::~LogHygieneScope() {
    detail::logger->setSink(_originalSink);
}

namespace {

// Forbidden substrings: a WARNING+ log line containing one of these signals data loss, a missing asset or an unmapped enum.
constexpr std::string_view kForbiddenPatterns[] = {
    "dropping ",             // table truncation, e.g. NPCTable/Localization/TransitionTable/SpellStats.
    "doesn't fit in a",      // the other half of the same truncation-warning shape.
    "Unknown monster spell", // unmapped monster spell enum.
    "not found",             // missing asset/sound/id lookup (covers "sound id {} not found" etc).
    "not loaded!",           // missing sprite (Sprites.cpp).
    "Unable to load ",       // missing image/PCX resource.
};

} // namespace

std::vector<std::string> LogHygieneScope::violations() const {
    std::vector<std::string> result;
    for (const std::string &message : messages()) {
        if (isLogHygieneAllowlisted(message))
            continue;
        for (std::string_view pattern : kForbiddenPatterns) {
            if (message.find(pattern) != std::string::npos) {
                result.push_back(message);
                break;
            }
        }
    }
    return result;
}

// Allowlist: known gaps that still warn. Each entry says why.
bool isLogHygieneAllowlisted(std::string_view message) {
    // MM8 lacks some directional-variant "spell70b*" sprites and has "null*" placeholder slots.
    if (message.starts_with("Sprite spell70b") || message.starts_with("Sprite null"))
        return true;

    // `SpellStats` skips spell ids past the regular spell range, MM8's extra spells have no `SpellId` yet.
    if (message.starts_with("SpellStats: dropping spell #"))
        return true;

    // MM8's sound table has no entry for id 1603, which an actor's bored-sound check requests.
    if (message == "AudioPlayer: sound id 1603 not found")
        return true;

    // 48: requested by `GUIWindow_Spellbook`'s constructor, absent from MM8's dsounds.bin (and from MM7's).
    // 1033: a monster's bored sound. MM8's dsounds.bin has no bored sound for any monster.
    for (int soundId : {48, 1033}) {
        if (message == fmt::format("AudioPlayer: sound id {} not found", soundId))
            return true;
    }

    // Missing from MM8's sound table, requested by a monster on Dagger Wound Island.
    if (message == "AudioPlayer: sound id 220 not found")
        return true;

    // Missing from MM8's sound table, requested in a monster fight.
    if (message == "AudioPlayer: sound id 222 not found")
        return true;

    // 225, 221: missing from MM8's sound table, requested by actors in Balthazar Lair.
    // The indoor sector lookup misses one point on the walk into Balthazar Lair.
    if (message == "AudioPlayer: sound id 221 not found")
        return true;
    if (message == "AudioPlayer: sound id 225 not found" ||message.starts_with("GetSector fail: "))
        return true;

    return false;
}

Pointi findClickableOpaquePixel(Ptr<GraphicsImage> texture, Pointi dstTopLeft, const Recti &clickableRegion) {
    if (!texture)
        return {-1, -1};
    RgbaImage &pixels = texture->rgba();
    for (int y = 0; y < pixels.height(); y++) {
        for (int x = 0; x < pixels.width(); x++) {
            if (pixels[{x, y}].a == 0)
                continue;
            Pointi candidate = dstTopLeft + Pointi(x, y);
            if (clickableRegion.contains(candidate))
                return candidate;
        }
    }
    return {-1, -1};
}
