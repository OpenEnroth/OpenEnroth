#include "LocationFunctions.h"

#include <cassert>

#include "Indoor.h"
#include "Outdoor.h"

LEVEL_TYPE uCurrentlyLoadedLevelType = LEVEL_NULL;

//----- (00450DA3) --------------------------------------------------------
bool GetAlertStatus() {
    int result;

    // TODO(captainurist): indoor & outdoor messed up, is this a bug?
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        result = pOutdoor->ddm.alertStatus;
    else
        result = uCurrentlyLoadedLevelType == LEVEL_OUTDOOR ? pIndoor->dlv.alertStatus : 0;

    return result;
}

LocationInfo &currentLocationInfo() {
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        return pIndoor->dlv;
    } else {
        assert(uCurrentlyLoadedLevelType == LEVEL_OUTDOOR);
        return pOutdoor->ddm;
    }
}

LocationTime &currentLocationTime() {
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        return pIndoor->stru1;
    } else {
        // assert(uCurrentlyLoadedLevelType == LEVEL_OUTDOOR); // TODO(captainurist): this triggers in tests now.
        return pOutdoor->loc_time;
    }
}
