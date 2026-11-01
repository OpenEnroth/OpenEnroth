#pragma once

#include <vector>

#include "Engine/Evt/EvtContext.h"
#include "Engine/Evt/EvtInstruction.h"
#include "Engine/Evt/EvtProgram.h"

#include "Library/Geometry/Vec.h"

/**
 * What an instruction tells the interpreter to do next.
 */
enum class EvtFlowType {
    EVT_FLOW_NEXT,  // Go on with the next step.
    EVT_FLOW_JUMP,  // Go on with the step in `EvtFlow::target`.
    EVT_FLOW_STOP,  // End the event.
    EVT_FLOW_YIELD, // The event pauses here, and the dialogue it opened resumes it at the next step if the player goes on.
};
using enum EvtFlowType;

/**
 * Where an event goes after one of its instructions ran.
 */
struct EvtFlow {
    EvtFlowType type = EVT_FLOW_NEXT;
    int target = 0; // Step to go on with, for `EVT_FLOW_JUMP`.
};

/**
 * Whether an event started a map transition.
 */
enum class EvtTransition {
    EVT_TRANSITION_NONE,    // The party stays on this map.
    EVT_TRANSITION_STARTED, // The map's leave events are due once the event ends.
};
using enum EvtTransition;

// EvtInterpreter
class EvtInterpreter {
 public:
     /**
      * @param context                  Event to run, and what it runs for.
      */
     void prepare(EvtContext context);

     /**
      * @param startStep                Step to start from.
      * @return                         Whether the event started a map transition.
      */
     [[nodiscard]] EvtTransition execute(int startStep);

     /**
      * Runs only the commands that decide whether an NPC's scripted topic is listed, `OnCanShowDialogItemCmp`,
      * `SetCanShowDialogItem` and `EndCanShowDialogItem`, and skips the rest.
      *
      * @return                         Whether the topic is listed. A script that never decides lists it.
      */
     [[nodiscard]] bool canShowTopic();

     [[nodiscard]] bool isValid();

     /**
      * @param ir                       Instruction to run.
      * @return                         Where the event goes after this instruction.
      */
     [[nodiscard]] EvtFlow executeInstruction(EvtInstruction ir);

 private:
     /**
      * @param step                     Step to run.
      * @return                         Step to run next, or -1 when the event is done.
      */
     [[nodiscard]] int executeStep(int step);

     /**
      * @param step                     Step to check.
      * @return                         Step to check next, or -1 once the script has decided.
      */
     [[nodiscard]] int checkTopicStep(int step);

     /**
      * @param step                     Step to resume from.
      * @return                         Continuation for the dialogue the current instruction opens.
      */
     [[nodiscard]] EvtContinuation continuation(int step) const;

     /**
      * Logs an error naming the event and step when the command's value is out of range for its variable.
      *
      * @param ir                       A compare, set, add or subtract command.
      * @return                         Whether the value is in range.
      */
     [[nodiscard]] bool validateVariableValue(const EvtInstruction &ir) const;

     /**
      * @param step                     Step to look up.
      * @return                         Instruction at that step, or `nullptr` if the event has none.
      */
     [[nodiscard]] const EvtInstruction *instructionAt(int step) const;

 private:
     EvtContext _context;
     std::vector<EvtInstruction> _events;
     bool _topicShown = true;
     bool _topicDecided = false;
     EvtTransition _transition = EVT_TRANSITION_NONE;
     EvtTargetCharacter _who = CHOOSE_PARTY;
};

void spawnMonsters(int16_t typeindex, int16_t level, int count,
    Vec3f pos, int group, int uUniqueName);
