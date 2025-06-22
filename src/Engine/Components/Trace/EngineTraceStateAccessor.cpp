#include "EngineTraceStateAccessor.h"

#include <string>

#include "Application/GameConfig.h"

#include "Engine/Party.h"
#include "Engine/Engine.h"
#include "Engine/MapInfo.h"
#include "Engine/mm7_data.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Trace/EventTrace.h"

#include "Utility/String/Ascii.h"

// TODO(captainurist): doesn't belong here
static std::string makeValidUtf8(const std::string& input) {
    std::string output;
    size_t i = 0;
    const size_t len = input.size();

    while (i < len) {
        unsigned char c = input[i];

        // Single-byte (ASCII)
        if (c <= 0x7F) {
            output += c;
            ++i;
        } else if ((c >> 5) == 0x6 && i + 1 < len &&          // 110xxxxx (2 bytes)
                 (input[i + 1] & 0xC0) == 0x80) {
            output += c;
            output += input[i + 1];
            i += 2;

        } else if ((c >> 4) == 0xE && i + 2 < len &&          // 1110xxxx (3 bytes)
                 (input[i + 1] & 0xC0) == 0x80 &&
                 (input[i + 2] & 0xC0) == 0x80) {
            output += c;
            output += input[i + 1];
            output += input[i + 2];
            i += 3;
        } else if ((c >> 3) == 0x1E && i + 3 < len &&         // 11110xxx (4 bytes)
                 (input[i + 1] & 0xC0) == 0x80 &&
                 (input[i + 2] & 0xC0) == 0x80 &&
                 (input[i + 3] & 0xC0) == 0x80) {
            output += c;
            output += input[i + 1];
            output += input[i + 2];
            output += input[i + 3];
            i += 4;
        } else {
            // Invalid UTF-8 byte sequence
            output += '?';
            ++i;
        }
    }

    return output;
}

static bool shouldSkip(const GameConfig *config, const ConfigSection *section, const AnyConfigEntry *entry) {
    return
        (section == &config->window && entry != &config->window.Width && entry != &config->window.Height) ||
        section == &config->graphics ||
        entry == &config->settings.MusicLevel ||
        entry == &config->settings.VoiceLevel ||
        entry == &config->settings.SoundLevel ||
        entry == &config->debug.LogLevel ||
        entry == &config->debug.NoVideo ||
        entry == &config->gameplay.NoPartyActorCollisions ||
        entry == &config->gameplay.QuickSavesCount;
}

static bool shouldTake(const GameConfig *config, const ConfigSection *section, const AnyConfigEntry *entry) {
    return
        entry->string() != entry->defaultString() ||
        entry == &config->debug.TraceFrameTimeMs ||
        entry == &config->debug.TraceRandomEngine ||
        entry == &config->debug.TraceNoVideo ||
        entry == &config->debug.TraceNoPartyActorCollisions;
}

void EngineTraceStateAccessor::prepareForRecording(GameConfig *config, ConfigPatch *patch) {
    *patch = ConfigPatch::fromConfig(config, [config] (const ConfigSection *section, const AnyConfigEntry *entry) {
        return !shouldSkip(config, section, entry) && shouldTake(config, section, entry);
    });

    config->graphics.FPSLimit.setValue(1000 / config->debug.TraceFrameTimeMs.value());
    config->debug.NoVideo.setValue(config->debug.TraceNoVideo.value());
    config->gameplay.NoPartyActorCollisions.setValue(config->debug.TraceNoPartyActorCollisions.value());
}

void EngineTraceStateAccessor::prepareForPlayback(GameConfig *config, const ConfigPatch &patch) {
    for (ConfigSection *section : config->sections())
        for (AnyConfigEntry *entry : section->entries())
            if (!shouldSkip(config, section, entry))
                entry->reset();

    // TODO(captainurist): Right now setting keybindings here doesn't work
    patch.apply(config);

    // We don't set voice & music levels to 0.0 b/c in this case our code doesn't even call into OpenAL, and this is NOT
    // what we want when playing traces, especially when running tests - we want the test code path to be the same as
    // in-game one.
    config->settings.MusicLevel.setValue(1.0);
    config->settings.VoiceLevel.setValue(1.0);
    config->settings.SoundLevel.setValue(1.0);
    config->window.MouseGrab.setValue(false);
    config->graphics.FPSLimit.setValue(0); // Unlimited.
    config->graphics.AlwaysCustomCursor.setValue(true); // We want to see the mouse pointer.
    config->debug.NoVideo.setValue(config->debug.TraceNoVideo.value());
    config->gameplay.NoPartyActorCollisions.setValue(config->debug.TraceNoPartyActorCollisions.value());
    pAudioPlayer->UpdateVolumeFromConfig();
}

EventTraceGameState EngineTraceStateAccessor::makeGameState() {
    auto toDebugString = [](const Item &item) {
        std::string result;
        if (item.isWand()) {
            result = fmt::format("{} [{}/{}]", item.GetIdentifiedName(), item.numCharges, item.maxCharges);
        } else if (item.isPotion() && item.itemId != ITEM_POTION_BOTTLE) {
            std::string name = item.GetIdentifiedName();
            if (name.ends_with("Potion")) {
                result = fmt::format("{} [{}]", name, item.potionPower);
            } else {
                result = fmt::format("{} Potion [{}]", name, item.potionPower);
            }
        } else if (item.isSpellScroll()) {
            result = fmt::format("Scroll of {}", item.GetIdentifiedName());
        } else if (item.standardEnchantment) {
            result = fmt::format("{} [+{}]", item.GetIdentifiedName(), item.standardEnchantmentStrength);
        } else if (item.itemId == ITEM_QUEST_LICH_JAR_FULL) {
            result = fmt::format("{} [{}]", item.GetIdentifiedName(), item.lichJarCharacterIndex);
        } else {
            result = item.GetIdentifiedName();
        }

        if (!item.IsIdentified())
            result += " [UNID]";
        if (item.IsBroken())
            result += " [BROKEN]";

        return makeValidUtf8(result);
    };

    EventTraceGameState result;
    result.locationName = ascii::toLower(pMapStats->pInfos[engine->_currentLoadedMapId].fileName);
    result.partyPosition = pParty->pos.toInt();
    for (const Character &character : pParty->pCharacters) {
        EventTraceCharacterState &traceCharacter = result.characters.emplace_back();
        traceCharacter.hp = character.health;
        traceCharacter.mp = character.mana;
        traceCharacter.might = character.GetActualMight();
        traceCharacter.intelligence = character.GetActualIntelligence();
        traceCharacter.personality = character.GetActualPersonality();
        traceCharacter.endurance = character.GetActualEndurance();
        traceCharacter.accuracy = character.GetActualAccuracy();
        traceCharacter.speed = character.GetActualSpeed();
        traceCharacter.luck = character.GetActualLuck();

        for (int index : character.pEquipment)
            if (index > 0)
                traceCharacter.equipment.emplace_back(toDebugString(character.pInventoryItemList[index - 1]));
        for (int index : character.pInventoryMatrix)
            if (index > 0)
                traceCharacter.backpack.emplace_back(toDebugString(character.pInventoryItemList[index - 1]));
    }
    return result;
}
