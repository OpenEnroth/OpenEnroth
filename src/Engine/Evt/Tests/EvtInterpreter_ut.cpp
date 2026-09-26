#include <array>
#include <cstdint>
#include <initializer_list>

#include "Testing/Game/GameTest.h"

#include "Engine/Evt/EvtInterpreter.h"
#include "Engine/Evt/EvtProgram.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Party.h"

static EvtInstruction forPlayer(EvtTargetCharacter who) {
    EvtInstruction result;
    result.opcode = EVENT_ForPartyMember;
    result.who = who;
    return result;
}

static EvtInstruction variableCommand(EvtOpcode opcode, EvtVariable variable, int value) {
    EvtInstruction result;
    result.opcode = opcode;
    result.data.variable_descr.type = variable;
    result.data.variable_descr.value = value;
    return result;
}

static void runEvent(std::initializer_list<EvtInstruction> instructions) {
    EvtProgram program;
    int step = 0;
    for (EvtInstruction ir : instructions) {
        ir.step = step++;
        program.add(1, ir);
    }

    EvtInterpreter interpreter;
    interpreter.prepare(program, 1, Pid(), false);
    interpreter.executeRegular(0);
}

GAME_TEST(EvtInterpreter, PartyVariableForAll) {
    // A command on a party variable after ForPlayer("All") changed it once per character, so four MM7 events in
    // global.evt moved the reputation four times as far as they said.
    game.startNewGame();
    int reputation = currentLocationInfo().reputation;
    runEvent({forPlayer(CHOOSE_PARTY), variableCommand(EVENT_Subtract, VAR_ReputationInCurrentLocation, 10)});
    EXPECT_EQ(currentLocationInfo().reputation, reputation - 10);
    runEvent({forPlayer(CHOOSE_PARTY), variableCommand(EVENT_Add, VAR_ReputationInCurrentLocation, 5)});
    EXPECT_EQ(currentLocationInfo().reputation, reputation - 5);

    std::array<uint64_t, 4> experience;
    for (int i = 0; i < 4; i++)
        experience[i] = pParty->pCharacters[i].experience;
    runEvent({forPlayer(CHOOSE_PARTY), variableCommand(EVENT_Add, VAR_Experience, 1000)});
    for (int i = 0; i < 4; i++)
        EXPECT_EQ(pParty->pCharacters[i].experience, experience[i] + 1000); // A character's variable still changes for everyone.
}

GAME_TEST(EvtInterpreter, SubtractGoldForAll) {
    // Subtract Gold after ForPlayer("All") took the price once per character until the party ran short, then ended
    // the event with the gold half gone.
    game.startNewGame();
    pParty->SetGold(250);
    runEvent({forPlayer(CHOOSE_PARTY), variableCommand(EVENT_Subtract, VAR_FixedGold, 100)});
    EXPECT_EQ(pParty->GetGold(), 150);

    int food = pParty->GetFood();
    runEvent({forPlayer(CHOOSE_PARTY), variableCommand(EVENT_Subtract, VAR_FixedGold, 200), variableCommand(EVENT_Add, VAR_FixedFood, 1)});
    EXPECT_EQ(pParty->GetGold(), 150);
    EXPECT_EQ(pParty->GetFood(), food); // A party that can't pay ends the event.
}
