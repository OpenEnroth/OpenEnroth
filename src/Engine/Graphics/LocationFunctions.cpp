#include "LocationFunctions.h"

#include <cassert>

#include "Indoor.h"
#include "Outdoor.h"

LEVEL_TYPE uCurrentlyLoadedLevelType = LEVEL_null;

//----- (00450DA3) --------------------------------------------------------
bool GetAlertStatus() {
    int result;

    // TODO(captainurist): indoor & outdoor messed up, is this a bug?
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        result = pOutdoor->ddm.alertStatus;
    else
        result = uCurrentlyLoadedLevelType == LEVEL_Outdoor ? pIndoor->dlv.alertStatus : 0;

    return result;
}

LocationInfo &currentLocationInfo() {
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        return pIndoor->dlv;
    } else {
        assert(uCurrentlyLoadedLevelType == LEVEL_Outdoor);
        return pOutdoor->ddm;
    }
}

LocationTime &currentLocationTime() {
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        return pIndoor->stru1;
    } else {
        // assert(uCurrentlyLoadedLevelType == LEVEL_Outdoor); // TODO(captainurist): this triggers in tests now.
        return pOutdoor->loc_time;
    }
}
