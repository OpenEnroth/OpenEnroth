#pragma once

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

bool IsTravelAvailable(int a1);

/**
 * @brief                               New function.
 *
 * @param schedule_id                   Index to transport_schedule.
 *
 * @return                              Number of days travel by transport will take with hireling modifiers.
 */
int GetTravelTimeTransportDays(int schedule_id);

void TravelByTransport();

// TODO(Nik-RE-dev): contain in cpp file
struct stru365_travel_info {
    unsigned char uMapInfoID;
    unsigned char pSchedule[7];
    unsigned int uTravelTime; // In days.
    int arrival_x;
    int arrival_y;
    int arrival_z;
    int arrival_view_yaw;
    unsigned int uQuestBit;  // quest bit required to set for this travel option
                             // to be enabled; otherwise 0
};
extern stru365_travel_info transport_schedule[35];
extern unsigned char transport_routes[20][4];
