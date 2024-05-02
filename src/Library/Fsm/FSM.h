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

using FSMTransitions = std::unordered_map<TransparentString, std::string, TransparentStringHash, TransparentStringEquals>;

class FSM : public FSMTransitionHandler, public FSMEventHandler {
 public:
    FSM();
    void setInitialState(std::string_view stateName);
    void update();
    [[nodiscard]] bool isDone() const;

    void addState(std::string_view name, std::unique_ptr<FSMState> state, FSMTransitions transitions);
    virtual void executeTransition(std::string_view transition) override;
    virtual void exitFromFSM() override;

 private:
    struct StateEntry {
        std::string name;
        std::unique_ptr<FSMState> state;
        FSMTransitions transitions;
    };

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

    std::unordered_map<TransparentString, std::unique_ptr<StateEntry>, TransparentStringHash, TransparentStringEquals> _states;
    StateEntry *_currentState{};
    StateEntry *_nextState{};
    StateEntry _nullState;
    bool _exitFromFSM{};

    static const LogCategory fsmLogCategory;
};
