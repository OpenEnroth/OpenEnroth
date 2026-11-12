#pragma once

#include "IBindings.h"

/**
 * Primitives under the MMExtension-compatible `evt` API that `scripts/mmext/core.lua` builds.
 */
class EvtBindings : public IBindings {
 public:
    virtual sol::table createBindingTable(sol::state_view &solState) const override;
};
