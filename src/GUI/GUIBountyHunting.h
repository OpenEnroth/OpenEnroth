#pragma once

#include <string>

void checkBountyRespawnAndAward();

/**
 * This function is called when you choose the "Bounty Hunt" dialogue option in a town hall.
 */
void discussBountyInTownhall();

// TODO(captainurist): bad API, improve
/**
 * @return                              Text to show in current town hall dialog.
 *                                      Must call `discussBountyInTownhall` first.
 */
std::string bountyHuntingText();
