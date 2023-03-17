#pragma once

#include <string>

// TODO(captainurist): looks like this one should be safe to drop.
/**
 * Opens a standalone bounty hunting dialogue AND presses the corresponding dialogue option right away. No idea how
 * to trigger it during gameplay.
 */
void openBountyHuntingDialogue();

/**
 * Handler for the "Bounty Hunt" dialogue option in a town hall.
 *
 * Regenerates bounty if needed, gives gold for a completed bounty hunt, and updates the current reply message to
 * be retrieved later with a call to `bountyHuntingText`.
 */
void bountyHuntingDialogueOptionClicked();

/**
 * @return                              Text to show after the player has clicked on the "Bounty Hunt" dialogue option.
 */
std::string bountyHuntingText();
