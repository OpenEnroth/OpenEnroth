#pragma once

#include <Utility/TransparentFunctors.h>
#include <Library/Logger/LogCategory.h>

#include "FSMEventHandler.h"
#include "FSMTransitionHandler.h"
#include "FSMState.h"

#include <memory>
#include <string_view>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

struct FSMTransitionTarget {
    std::string stateName;
    std::function<bool()> condition{};
};

using FSMTransitions = std::unordered_map<TransparentString, std::vector<FSMTransitionTarget>, TransparentStringHash, TransparentStringEquals>;

class FSM : public FSMTransitionHandler, public FSMEventHandler {
 public:
    FSM();

    /*
    * @brief Update the FSM current state or execute any pending transition.
    */
    void update();

    /*
    * @brief Check if the FSM has reached its internal _Exit state. When the FSM reach the _Exit state it means 
    */
    [[nodiscard]] bool hasReachedExitState() const;

    virtual void executeTransition(std::string_view transition) override;
    virtual void exitFromFSM() override;

    /*
    * @brief Set the next state to be reached in the FSM. The transition won't occur immediately but will be executed during the next FSM::update() call.
    * The jumpToState function does not require a previously defined transition connecting the current state to the target state. The jump is unconditional.
    * Since the actual transition occurs during the next FSM::update() call, subsequent calls to FSM::jumpToState, FSM::executeTransition, or FSM::exitFromFSM
    * will overwrite the target state.
    * @param stateName The name of the state to transition to. This name must belong to a state that has been previously added through FSM::addState.
    */
    void jumpToState(std::string_view stateName);

    struct StateEntry {
        std::string name;
        std::unique_ptr<FSMState> state;
        FSMTransitions transitions;
    };

    void addState(std::unique_ptr<StateEntry> stateEntry);

 private:
    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual bool mouseMoveEvent(const PlatformMouseEvent *event) override;
    virtual bool mousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool mouseReleaseEvent(const PlatformMouseEvent *event) override;
    virtual bool wheelEvent(const PlatformWheelEvent *event) override;
    virtual bool moveEvent(const PlatformMoveEvent *event) override;
    virtual bool resizeEvent(const PlatformResizeEvent *event) override;
    virtual bool activationEvent(const PlatformWindowEvent *event) override;
    virtual bool closeEvent(const PlatformWindowEvent *event) override;
    virtual bool gamepadConnectionEvent(const PlatformGamepadEvent *event) override;
    virtual bool gamepadKeyPressEvent(const PlatformGamepadKeyEvent *event) override;
    virtual bool gamepadKeyReleaseEvent(const PlatformGamepadKeyEvent *event) override;
    virtual bool gamepadAxisEvent(const PlatformGamepadAxisEvent *event) override;
    virtual bool nativeEvent(const PlatformNativeEvent *event) override;
    virtual bool textInputEvent(const PlatformTextInputEvent *event) override;

    StateEntry *_getStateByName(std::string_view stateName);
    void _createDefaultStates();

    std::unordered_map<TransparentString, std::unique_ptr<StateEntry>, TransparentStringHash, TransparentStringEquals> _states;
    StateEntry *_currentState{};
    StateEntry *_nextState{};
    bool _hasReachedExitState{};

    static const LogCategory fsmLogCategory;
};
