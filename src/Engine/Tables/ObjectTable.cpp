#include "ObjectTable.h"

ObjectTable *pObjectTable;

unsigned int ObjectTable::ObjectIDByItemID(SpriteId uItemID) {
    for (size_t i = 0; i < pObjects.size(); i++)
        if (uItemID == pObjects[i].uObjectID)
            return i;
    return 0;
}
