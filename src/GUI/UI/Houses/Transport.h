#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

class GUIWindow_Transport : public GUIWindow_House {
 public:
    explicit GUIWindow_Transport(HouseId houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_Transport() {}

    virtual void houseDialogueOptionSelected(DialogueId option) override;
    virtual void houseSpecificDialogue() override;
    virtual std::vector<DialogueId> listDialogueOptions() override;
    virtual void updateDialogueOnEscape() override;

 protected:
    void mainDialogue();
    void transportDialogue();

    /**
     * Ends the wait after the party has paid, and starts the map change if the route leaves the map.
     */
    void depart();

 private:
    /**
     * @brief                               New function.
     *
     * @param schedule_id                   Index to transport_schedule.
     *
     * @return                              Number of days travel by transport will take with hireling modifiers.
     */
    int getTravelTimeTransportDays(int schedule_id);

    std::optional<int64_t> _departureTime; // Platform tick count at which the paid party leaves.
};

bool isTravelAvailable(HouseId houseId);
