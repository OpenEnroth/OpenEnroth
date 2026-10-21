#include "Engine/Tables/DecorationTable.h"

#include "Utility/String/Ascii.h"

DecorationTable *pDecorationTable;

DecorationId DecorationTable::decorationId(std::string_view name) const {
    if (name.empty())
        return DECORATION_NULL;

    for (size_t i = 1; i < decorations.size(); ++i)
        if (ascii::noCaseEquals(name, decorations[i].internalName))
            return static_cast<DecorationId>(i);

    return DECORATION_NULL;
}
