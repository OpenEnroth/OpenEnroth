#include "AwardEnums.h"

#include "Utility/Segment.h"

inline Segment<AwardId> allAwards() {
    return {AWARD_FIRST, AWARD_LAST};
}
