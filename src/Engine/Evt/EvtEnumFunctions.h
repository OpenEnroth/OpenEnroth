#include "EvtEnums.h"

#include <cassert>
#include <utility>

inline int historyIndex(EvtVariable variable) {
    assert(variable >= VAR_History_0 && variable <= VAR_History_28);
    return std::to_underlying(variable) - std::to_underlying(VAR_History_0);
}
