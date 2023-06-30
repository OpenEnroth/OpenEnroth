#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "Utility/IndexedArray.h"

#include "Engine/Objects/NPCEnums.h"

struct NPCData;

// TODO(Nik-RE-dev): remove
//----- (0047730C) --------------------------------------------------------
inline bool CheckPortretAgainstSex(int a1, int) { return true; }

bool PartyHasDragon();
bool CheckHiredNPCSpeciality(NPCProf prof);

int UseNPCSkill(NPCProf profession, int id);

const std::string &GetProfessionActionText(NPCProf prof);
NPCData *GetNPCData(signed int npcid);
NPCData *GetNewNPCData(signed int npcid, int *npc_indx);
int GetGreetType(signed int SpeakingNPC_ID);

class FlatHirelings {
 public:
    /**
     * Populates this structure.
     */
    void Prepare();

    /**
     * @return                          Total number of hirelings in this structure.
     */
    size_t Size() const {
        return count;
    }

    /**
     * @param index                     Hireling index.
     * @return                          Whether a hireling is actually a follower (doesn't count towards the total
     *                                  hireling count, e.g. Margaret the guide on Emerald island).
     */
    bool IsFollower(size_t index) const;

    /**
     * @param index                     Hireling index.
     * @return                          Associated `NPCData`.
     */
    NPCData *Get(size_t index) const;

 private:
    /** Hireling / follower NPC ids.
     * If 0 or 1, then it's an index into `pParty->pHirelings`.
     * If 2 or more, then you have to subtract 2 to get an index into `pNPCStats->pNewNPCData`. */
    std::array<int8_t, 1024> ids = {{}};

    /** Number of valid ids in the array above. */
    size_t count = 0;
};
