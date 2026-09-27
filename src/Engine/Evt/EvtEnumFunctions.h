#include "EvtEnums.h"

#include <cassert>
#include <utility>

inline int historyIndex(EvtVariable variable) {
    assert(variable >= VAR_History_0 && variable <= VAR_History_28);
    return std::to_underlying(variable) - std::to_underlying(VAR_History_0);
}

/**
 * @param variable                      Evt variable.
 * @return                              Whether the variable belongs to the party or to the world rather than to a
 *                                      character, so that `Add`, `Subtract` and `Set` change it once whoever they
 *                                      target. Vanilla MM7 changes it once for every character that the command
 *                                      targets.
 */
inline bool isPartyVariable(EvtVariable variable) {
    if (variable >= VAR_MapPersistentVariable_0 && variable <= VAR_MapPersistentDecorVariable_24)
        return true;
    if (variable >= VAR_Counter1 && variable <= VAR_Counter10)
        return true;
    if (variable >= VAR_UnknownTimeEvent0 && variable <= VAR_UnknownTimeEvent19)
        return true;
    if (variable >= VAR_History_0 && variable <= VAR_History_28)
        return true;

    switch (variable) {
        case VAR_QBits_QuestsDone:
        case VAR_FixedGold:
        case VAR_RandomGold:
        case VAR_FixedFood:
        case VAR_RandomFood:
        case VAR_AutoNotes:
        case VAR_NPCs2:
        case VAR_HiredNPCHasSpeciality:
        case VAR_ReputationInCurrentLocation:
        case VAR_GoldInBank:
        case VAR_NumDeaths:
        case VAR_NumBounties:
        case VAR_PrisonTerms:
        case VAR_ArenaWinsPage:
        case VAR_ArenaWinsSquire:
        case VAR_ArenaWinsKnight:
        case VAR_ArenaWinsLord:
            return true;
        default:
            return false;
    }
}
