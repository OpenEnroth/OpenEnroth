#pragma once

#include <cstdio>
#include <array>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Objects/NPCEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/PartyEnums.h"

#include "Utility/IndexedArray.h"
#include "Utility/String/Format.h"

// TODO(captainurist): #enum
#define LSTR_AC                               0   // "AC"
#define LSTR_ACCURACY                         1   // "Accuracy"
#define LSTR_AFRAID                           4   // "Afraid"
#define LSTR_AGE                              5   // "Age"
#define LSTR_AIR                              6   // "Air"
#define LSTR_OBELISK_NOTES                    8   // "Obelisk Notes"
#define LSTR_CONGRATULATIONS                  9   // "Congratulations!"
#define LSTR_TOWN_PORTAL                     10   // "Town Portal"
#define LSTR_ARMOR                           11   // "Armor"
#define LSTR_ARMOR_CLASS                     12   // "Armor Class"
#define LSTR_AUTOSAVE                        16   // "Autosave"
#define LSTR_EXP                             17   // "Exp."
#define LSTR_ATTACK                          18   // "Attack"
#define LSTR_AVAILABLE                       19   // "Available"
#define LSTR_AVAILABLE_SKILLS                20   // "Available Skills"
#define LSTR_AWARDS                          22   // "Awards"
#define LSTR_AWARDS_FOR                      23   // "Awards for"
#define LSTR_BALANCE                         25   // "Balance"
#define LSTR_BODY                            29   // "Body"
#define LSTR_BONUS                           30   // "Bonus"
#define LSTR_BROKEN_ITEM                     32   // "Broken Item"
#define LSTR_CANCEL                          34   // "Cancel"
#define LSTR_TOWN_PORTAL_TO_S                35   // "Town Portal to %s"
#define LSTR_FMT_S_CANT_BE_USED_THIS_WAY     36   // "%s can not be used that way"
#define LSTR_TOTAL_TIME                      37   // "Total Time:"
#define LSTR_CAST_SPELL                      38   // "Cast Spell"
#define LSTR_CHOOSE_TARGET                   39   // "Choose target"
#define LSTR_CLASS                           41   // "Class"
#define LSTR_COND                            45   // "Cond"
#define LSTR_CONDITION                       47   // "Condition"
#define LSTR_CREATE_PARTY_FANCY              51   // "C R E A T E   P A R T Y"
#define LSTR_DAMAGE                          53   // "Damage"
#define LSTR_DAWN                            55   // "Dawn"
#define LSTR_DAY_CAPITALIZED                 56   // "Day"
#define LSTR_DAYS                            57   // "Days"
#define LSTR_DEPOSIT                         60   // "Deposit"
#define LSTR_PRESS_ESCAPE                    61   // "Press Escape"
#define LSTR_PLEASE_REINSTALL                63   // "Might and Magic VII is having trouble loading files. Please re-install to fix this problem. Note: Re-installing will not destroy your save games."
#define LSTR_DETAIL_TOGGLE                   64   // "Detail Toggle"
#define LSTR_DMG                             66   // "Dmg"
#define LSTR_FMT_S_DOES_NOT_HAVE_SKILL       67   // "%s does not have the skill"
#define LSTR_DONATE                          68   // "Donate"
#define LSTR_EARTH                           70   // "Earth"
#define LSTR_EMPTY_SAVESLOT                  72   // "Empty"
#define LSTR_ENTER                           73   // "Enter"
#define LSTR_END_CONVERSATION                74   // "End Conversation"
#define LSTR_ENDURANCE                       75   // "Endurance"
#define LSTR_CALENDAR                        78   // "Calendar"
#define LSTR_DIALOGUE_EXIT                   79   // "Exit"
#define LSTR_EXIT_BUILDING                   80   // "Exit Building"
#define LSTR_EXIT_REST                       81   // "Exit Rest"
#define LSTR_EXIT_GAME_PROMPT                82   // "Are you sure?  Click again to quit"
#define LSTR_EXPERIENCE                      83   // "Experience"
#define LSTR_FAME                            84   // "Fame"
#define LSTR_POTION_NOTES                    85   // "Potion Notes"
#define LSTR_FMT_BUY_D_FOOD_FOR_D_GOLD       86   // "Fill Packs to %d days for %d gold"
#define LSTR_FIRE                            87   // "Fire"
#define LSTR_GAME_OPTIONS                    93   // "Game Options"
#define LSTR_FMT_YOUR_SCORE_D                94   // "Your score: %lu"
#define LSTR_GRAND                           96   // "Grand"
#define LSTR_GOLD                            97   // "Gold"
#define LSTR_RACE_HUMAN                      99   // "Human"
#define LSTR_RACE_DWARF                     101  // "Dwarf"
#define LSTR_RACE_ELF                       103  // "Elf"
#define LSTR_HEAL                           104  // "Heal"
#define LSTR_RACE_GOBLIN                    106  // "Goblin"
#define LSTR_HP                             107  // "HP"
#define LSTR_HIT_POINTS                     108  // "Hit Points"
#define LSTR_HOUR                           109  // "Hour"
#define LSTR_HOURS                          110  // "Hours"
#define LSTR_HOW_MUCH                       112  // "How Much?"
#define LSTR_IDENTIFY                       113  // "Identify"
#define LSTR_INTELLECT                      116  // "Intellect"
#define LSTR_WE_HOPE_YOU_ENJOYED_MM7        118  // "We hope that you've enjoyed playing Might and Magic VII as much as we did making it. We have saved this screen as MM7_WIN.PCX in your MM7 directory. You can print it out as proof of your accomplishment."
#define LSTR_INVENTORY                      120  // "Inventory"
#define LSTR_PERMANENT                      121  // "Permanent"
#define LSTR_JOIN                           122  // "Join"
#define LSTR_MISC_NOTES                     123  // "Miscellaneous Notes"
#define LSTR_FMT_DO_YOU_WISH_TO_LEAVE_S     126  // "Do you wish to leave %s?"
#define LSTR_NO_TEXT                        127  // "No Text!"
#define LSTR_FMT_IT_TAKES_D_DAYS_TO_S       128  // "It will take %d days to travel to %s."
#define LSTR_FMT_S_THE_LEVEL_D_S            129  // "%s the Level %u %s"
#define LSTR_LEVEL                          131  // "Level"
#define LSTR_YEARS                          132  // "Years"
#define LSTR_STANDARD                       134  // "Standard"
#define LSTR_LOADING                        135  // "Loading"
#define LSTR_LUCK                           136  // "Luck"
#define LSTR_FOUNTAIN_NOTES                 137  // "Fountain Notes"
#define LSTR_MAGIC                          138  // "Magic"
#define LSTR_MAPS                           139  // "Maps"
#define LSTR_RATIONS_FULL                   140  // "Your packs are already full!"
#define LSTR_SEER_NOTES                     141  // "Seer Notes"
#define LSTR_MIND                           142  // "Mind"
#define LSTR_MISC                           143  // "Misc"
#define LSTR_MIGHT                          144  // "Might"
#define LSTR_MONTH                          146  // "Month"
#define LSTR_MONTHS                         148  // "Months"
#define LSTR_ELIGIBLE_TO_LEVELUP            147  // "You are eligible to train to %u."
#define LSTR_NAME                           149  // "Name"
#define LSTR_CONGRATULATIONS_ADVENTURER     151  // "Congratulations Adventurer."
#define LSTR_SPECIAL                        152  // "Special"
#define LSTR_NONE                           153  // "None"
#define LSTR_AUTONOTES                      154  // "Autonotes"
#define LSTR_NOT_ENOUGH_GOLD                155  // "You don't have enough gold"
#define LSTR_STAY_IN_THIS_AREA              156  // "Stay in this Area"
#define LSTR_3DO_COPYRIGHT                  157  // ""© 1999 The 3DO Company.All Rights
                                                 // Reserved.Might and Magic, Bloodand Honor, New World Computing, 3DO, and their
                                                 // respective logos, are trademarksand /or service marks of The 3DO Company in the
                                                 // U.S. and other countries.All other trademarks belong to their respective
                                                 // owners.New World Computing is a division of The 3DO Company.""
#define LSTR_DISPLAY                        159  // "Display"
#define LSTR_LEARN_SKILLS                   160  // "Learn Skills"
#define LSTR_PARALYZED                      162  // "Paralyzed"
#define LSTR_PERSONALITY                    163  // "Personality"
#define LSTR_FMT_S_HITS_S_FOR_U             164  // "%s hits %s for %lu points"
#define LSTR_PLEASE_WAIT                    165  // "Please Wait"
#define LSTR_THE_MM7_DEV_TEAM               167  // "- The Might and Magic VII Development Team."
#define LSTR_POINTS                         168  // "Points"
#define LSTR_QSPELL                         170  // "QSpell"
#define LSTR_QUICK_SPELL                    172  // "Quick Spell"
#define LSTR_QUICK_REFERENCE                173  // "Quick Reference"
#define LSTR_CURRENT_QUESTS                 174  // "Current Quests"
#define LSTR_FMT_S_INFLICTS_U_KILLING_S     175  // "%s inflicts %lu points killing %s"
#define LSTR_FMT_RENT_ROOM_FOR_D_GOLD       178  // "Rent Room for %d gold"
#define LSTR_REPAIR                         179  // "Repair"
#define LSTR_REPUTATION                     180  // "Reputation"
#define LSTR_REST                           182  // "Rest"
#define LSTR_REINSTALL_NECESSARY            184  // "Reinstall Necessary"
#define LSTR_TIME_IN_ERATHIA                186  // "Time in Erathia"
#define LSTR_STEAL_ITEM_FMT                 181  // "Steal %24"
#define LSTR_REST_AND_HEAL_8_HOURS          183  // "Rest & Heal 8 Hours"
#define LSTR_STEAL_ITEM                     185  // "Steal item"
#define LSTR_STOLEN                         187  // "Stolen"
#define LSTR_FMT_S_SHOOTS_S_FOR_U           189  // "%s shoots %s for %lu points"
#define LSTR_BANNED_FROM_SHOP               191  // "You've been banned from this shop!"
#define LSTR_SCROLL_UP                      192  // "Scroll Up"
#define LSTR_SCROLL_DOWN                    193  // "Scroll Down"
#define LSTR_SELECT_ITEM_TO_BUY             195  // "Select the Item to Buy"
#define LSTR_SELECT_ITEM_TO_IDENTIFY        197  // "Select the Item to Identify"
#define LSTR_SELECT_ITEM_TO_REPAIR          198  // "Select the Item to Repair"
#define LSTR_SELECT_ITEM_TO_SELL            199  // "Select the Item to Sell"
#define LSTR_SELL                           200  // "Sell"
#define LSTR_START_NEW_GAME_PROMPT          201  // "Are you sure?  Click again to start a New Game"
#define LSTR_SHOOT                          203  // "Shoot"
#define LSTR_SKILLS                         205  // "Skills"
#define LSTR_SKILLS_FOR                     206  // "Skills for"
#define LSTR_SKILL_POINTS                   207  // "Skill Points"
#define LSTR_SP                             209  // "SP"
#define LSTR_SPECIAL_2                      210  // "Special"
#define LSTR_SPEED                          211  // "Speed"
#define LSTR_SPELL_POINTS                   212  // "Spell Points"
#define LSTR_STATS                          216  // "Stats"
#define LSTR_STONED                         220  // "Stoned"
#define LSTR_FATE                           221  // "Fate"
#define LSTR_GRANDMASTER                    225  // "Grandmaster"
#define LSTR_GAMMA_DESCRIPTION              226  // ""Gamma controls the relative ""brightness"" of the game.May vary depending on your monitor.""
#define LSTR_HAMMERHANDS                    228  // "Hammerhands"
#define LSTR_PAIN_REFLECTION                229  // "Pain Reflection"
#define LSTR_NOT_IDENTIFIED                 232  // "Not Identified"
#define LSTR_WAIT_WITHOUT_HEALING           236  // "Wait without healing"
#define LSTR_WAIT_UNTIL_DAWN                237  // "Wait until Dawn"
#define LSTR_WAIT_5_MINUTES                 238  // "Wait 5 Minutes"
#define LSTR_WAIT_1_HOUR                    239  // "Wait 1 Hour"
#define LSTR_WATER                          240  // "Water"
#define LSTR_WEAPONS                        242  // "Weapons"
#define LSTR_WITHDRAW                       244  // "Withdraw"
#define LSTR_YEAR                           245  // "Year"
#define LSTR_ZOOM_IN                        251  // "Zoom In"
#define LSTR_ZOOM_OUT                       252  // "Zoom Out"
#define LSTR_ARCOMAGE_CARD_DISCARD          266  // "DISCARD A CARD"
#define LSTR_SHIELD                         279  // "Shield"
#define LSTR_OFFICIAL                       304  // "Official"
#define LSTR_FMT_S_STOLE_D_GOLD             302  // "%s stole %d gold!"
#define LSTR_SET_BEACON                     375  // "Set Beacon"
#define LSTR_FMT_S_WAS_CAUGHT_STEALING      376  // "%s was caught stealing!"
#define LSTR_FMT_S_FAILED_TO_STEAL          377  // "%s failed to steal anything!"
#define LSTR_FMT_S_D_D                      378  // ""%s %d, %d""
#define LSTR_REPUTATION_HATED               379  // "Hated"
#define LSTR_FMT_YOU_ALREADY_KNOW_S_SPELL   380  // "You already know the %s spell"
#define LSTR_FMT_DONT_HAVE_SKILL_TO_LEAN_S  381  // "You don't have the skill to learn %s"
#define LSTR_FMT_THAT_PLAYER_IS_S           382  // "That player is %s"
#define LSTR_HIS                            383  // "his"
#define LSTR_HER                            384  // "her"
#define LSTR_SIR_LOWERCASE                  385  // "sir"
#define LSTR_SIR                            386  // "Sir"
#define LSTR_LADY_LOWERCASE                 387  // "lady"
#define LSTR_LORD                           388  // "Lord"
#define LSTR_LADY                           389  // "Lady"
#define LSTR_BROTHER                        390  // "brother"
#define LSTR_SISTER                         391  // "sister"
#define LSTR_REPUTATION_UNFRIENDLY          392  // "Unfriendly"
#define LSTR_DAUGHTER                       393  // "daughter"
#define LSTR_UNKNOWN                        394  // "Unknown"
#define LSTR_MORNING                        395  // "morning"
#define LSTR_DAY                            396  // "day"
#define LSTR_EVENING                        397  // "evening"
#define LSTR_REPUTATION_NEUTRAL             399  // "Neutral"
#define LSTR_BUY_SPELLS                     400  // "Buy Spells"
#define LSTR_FMT_SKILL_COST_D               401  // "Skill Cost: %lu"
#define LSTR_REPUTATION_FRIENDLY            402  // "Friendly"
#define LSTR_FMT_ALREADY_KNOW_THE_S_SKILL   403  // "You already know the %s skill"
#define LSTR_FMT_D_DAYS_TO_S                404  // "%d days to %s"
#define LSTR_FMT_TRAVEL_COST_D_GOLD         405  // "Travel Cost %d gold"
#define LSTR_HIRE                           406  // "Hire"
#define LSTR_MORE_INFORMATION               407  // "More Information"
#define LSTR_HIRE_RELEASE                   408  // "Release %s"
#define LSTR_FMT_DO_YOU_WISH_TO_LEAVE_S_2   409  // "Do you wish to leave %s?"
#define LSTR_FMT_LEAVE_S                    410  // "Leave %s"
#define LSTR_FMT_ENTER_S                    411  // "Enter %s"
                                                 // "Войти в ^Pv[%s]"
#define LSTR_PARTY_UNASSIGNED_POINTS        412  // "Create Party cannot be completed unless you have assigned all characters 2 extra skills and have spent all of your bonus points."
#define LSTR_PARTY_TOO_MUCH_POINTS          413  // "You can't spend more than 50 points."
#define LSTR_FMT_OPEN_TIME                  414  // "This place is open from %d%s to %d%s"
#define LSTR_FMT_S_IS_IN_NO_CODITION_TO_S   427  // "%s is in no condition to %s"
#define LSTR_SPELL_FAILED                   428  // "Spell failed"
#define LSTR_FMT_S_THE_S                    429  // "%s the %s"
                                                 // "^Pi[%s] %s"
#define LSTR_FMT_S_NOW_LEVEL_D              430  // "%s is now Level %lu and has earned %lu Skill Points!"
#define LSTR_NORMAL                         431  // "Normal"
#define LSTR_MASTER                         432  // "Master"
#define LSTR_EXPERT                         433  // "Expert"
#define LSTR_REPUTATION_RESPECTED           434  // "Respected"
#define LSTR_FMT_CONVERSE_WITH_S            435  // "Converse with %s"
#define LSTR_MINUTES                        436  // "Minutes"
#define LSTR_MINUTE                         437  // "Minute"
#define LSTR_SECONDS                        438  // "Seconds"
#define LSTR_SECOND                         439  // "Second"
#define LSTR_HEROISM                        440  // "Heroism"
#define LSTR_HASTE                          441  // "Haste"
#define LSTR_STONESKIN                      442  // "Stoneskin"
#define LSTR_BLESS                          443  // "Bless"
#define LSTR_OOPS                           444  // "Ooops!"
#define LSTR_IDENTIFY_FAILED                446  // "Identify Failed"
#define LSTR_REPAIR_FAILED                  448  // "Repair Failed"
#define LSTR_POWER                          449  // "Power"
#define LSTR_FMT_ACTIVE_SPELLS_S            450  // "Active Spells: %s"
#define LSTR_ACTIVE_PARTY_SPELLS            451  // "Active Party Spells"
#define LSTR_FMT_TYPE_S                     463  // "Type: %s"
#define LSTR_CHARGES                        464  // "Charges"
#define LSTR_VALUE                          465  // "Value"
#define LSTR_FMT_YOU_FOUND_D_GOLD_FOLLOWERS 466  // "You found %lu gold (followers take %lu)!"
#define LSTR_FMT_YOU_FOUND_D_GOLD           467  // "You found %lu gold!"
#define LSTR_FMT_CLICKING_WILL_SPEND_POINTS 468  // "Clicking here will spend %d Skill Points"
#define LSTR_FMT_NEED_MORE_SKILL_POINTS     469  // "You need %d more Skill Points to advance here"
#define LSTR_FMT_GET_S                      470  // "Get %s"
#define LSTR_FMT_YOU_FOUND_ITEM             471  // "You found an item(% s)!"
                                                 // "Вы нашли ^Pv[%s]!"
#define LSTR_FMT_RECALL_TO_S                474  // "Recall to %s"
#define LSTR_FMT_SET_S_OVER_S               475  // "Set %s over %s"
#define LSTR_FMT_SET_S_TO_S                 476  // "Set %s to %s"
#define LSTR_ALREADY_RESTING                477  // "You are already resting!"
#define LSTR_CANT_REST_IN_TURN_BASED        478  // "You can't rest in turn-based mode!"
#define LSTR_CANT_REST_HERE                 479  // "You can't rest here!"
#define LSTR_HOSTILE_ENEMIES_NEARBY         480  // "There are hostile enemies near!"
#define LSTR_ENCOUNTER                      481  // "Encounter!"
#define LSTR_NOT_ENOUGH_FOOD                482  // "You don't have enough food to rest"
#define LSTR_FMT_SET_S_AS_READY_SPELL       483  // "Set %s as the Ready Spell"
#define LSTR_CLICK_TO_SET_QUICKSPELL        484  // "Select a spell then click here to set a QuickSpell"
#define LSTR_CAST_S                         485  // "Cast %s"
#define LSTR_SELECT_S                       486  // "Select %s"
#define LSTR_SKILL_ALREADY_MASTERED         487  // "You have already mastered this skill!"
#define LSTR_NOT_ENOUGH_SKILL_POINTS        488  // "You don't have enough skill points!"
#define LSTR_FMT_D_TOTAL_GOLD_D_IN_BANK     489  // ""You have %d total gold, %d in the Bank""
#define LSTR_FMT_YOU_FOUND_GOLD_AND_ITEM    490  // "You found %d gold and an item(%s)!"
                                                 // "Вы нашли ^I[%d] золот^L[ой;ых;ых] и предмет (%s)!"
#define LSTR_CANT_METEOR_SHOWER_INDOORS     491  // "Can't cast Meteor Shower indoors!"
#define LSTR_CANT_INFERNO_OUTDOORS          492  // "Can't cast Inferno outdoors!"
#define LSTR_CANT_JUMP_AIRBORNE             493  // "Can't cast Jump while airborne!"
#define LSTR_CANT_FLY_INDOORS               494  // "Can't fly indoors"
#define LSTR_CANT_STARBURST_INDOORS         495  // "Can't cast Starburst indoors!"
#define LSTR_NO_VALID_SPELL_TARGET          496  // "No valid target exists!"
#define LSTR_CANT_PRISMATIC_OUTDOORS        497  // "Can't cast Prismatic Light outdoors!"
#define LSTR_CANT_ARMAGEDDON_INDOORS        499  // "Can't cast Armageddon indoors!"
#define LSTR_FMT_YOU_HAVE_D_GOLD            500  // "You have %lu gold"
#define LSTR_FMT_YOU_HAVE_D_FOOD            501  // "You have %lu food"
#define LSTR_FMT_YOU_FIND_D_FOOD            502  // "You find %lu food"
#define LSTR_FMT_YOU_LOSE_D_GOLD            503  // "You lose %lu gold"
#define LSTR_FMT_YOU_LOSE_D_FOOD            504  // "You lose %lu food"
#define LSTR_READING                        505  // "Reading..."
#define LSTR_PC_NAME_RODERIC                506  // "Roderic"
#define LSTR_PC_NAME_ALEXIS                 507  // "Alexis"
#define LSTR_PC_NAME_SERENA                 508  // "Serena"
#define LSTR_PC_NAME_ZOLTAN                 509  // "Zoltan"
#define LSTR_NOTHING_HERE                   521  // "Nothing here"
#define LSTR_SP_COST                        522  // "SP Cost"
#define LSTR_RECALL_BEACON                  523  // "Recall Beacon"
#define LSTR_CHEATED_THE_DEATH              524  // "Once again you've cheated death! ..."
#define LSTR_TIME                           526  // "Time"
#define LSTR_THANK_YOU                      527  // "Thank You!"
#define LSTR_NO_FURTHER_OFFERS              528  // "I can offer you nothing further."
#define LSTR_CANT_TRAIN_FURTHER             529  // ""Sorry, but we are unable to train you.""
#define LSTR_MOON                           530  // "Moon"
#define LSTR_LOCATION                       531  // "Location"
#define LSTR_PLEASE_TRY_BACK_IN             532  // "Please try back in "
#define LSTR_HIRE_NO_ROOM                   533  // "I cannot join you, you're party is full"
#define LSTR_FMT_BECOME_S_IN_S_FOR_D_GOLD   534  // "Become %s in %s for %lu gold"
                                                 // "Получить степень ^Pr[%s] в навыке ^Pr[%s] за ^I[%lu] золот^L[ой;ых;ых]"
#define LSTR_LEARN                          535  // "Learn"
#define LSTR_TEACHER_LEVEL_TOO_LOW          536  // ""With your skills, you should be working here as a teacher.""
#define LSTR_FMT_TRAIN_LEVEL_D_FOR_D_GOLD   537  // "Train to level %d for %d gold"
#define LSTR_XP_UNTIL_NEXT_LEVEL            538  // "You need %d more experience to train to level %d"
#define LSTR_IDENTIFY_ITEMS                 541  // "Identify Items"
#define LSTR_FMT_SEEK_KNOWLEDGE_ELSEWHERE   544  // "Seek knowledge elsewhere %s the %s"
#define LSTR_COME_BACK_ANOTHER_DAY          561  // ""Sorry, come back another day""
#define LSTR_DO_ANYTHING                    562  // "do anythig"
#define LSTR_INVENTORY_IS_FULL              563  // "Pack is Full!"
#define LSTR_DUSK                           566  // "Dusk"
#define LSTR_NIGHT                          567  // "Night"
#define LSTR_DONE                           569  // "Done!"
#define LSTR_GOOD_AS_NEW                    570  // "Good as New!"
#define LSTR_SCROLL_LEFT                    572  // "Scroll Left"
#define LSTR_SCROLL_RIGHT                   573  // "Scroll Right"
#define LSTR_ARENA_WELCOME                  574  // "Welcome to the Arena of Life and Death.  Remember,
                                                 // you are only allowed one arena combat per visit.  To
                                                 // fight an arena battle, select the option that best
                                                 // describes your abilities and return to me- if you
                                                 // survive"
#define LSTR_PLEASE_WAIT_WHILE_I_SUMMON     575  // "Please wait while I summon the monsters.  Good luck."
#define LSTR_ARENA_REWARD                   576  // "Congratulations on your win. Here's your stuff: %u gold."
#define LSTR_ARENA_PREMATURE_EXIT           577  // "Get back in there you wimps"
#define LSTR_ARENA_DIFFICULTY_PAGE          578  // "Page"
#define LSTR_ARENA_DIFFICULTY_SQUIRE        579  // "Squire"
#define LSTR_ARENA_DIFFICULTY_KNIGHT        580  // "Knight"
#define LSTR_ARENA_DIFFICULTY_LORD          581  // "Lord"
#define LSTR_ARENA_AREADY_WON               582  // "You already won this trip to the Arena
#define LSTR_NO_SAVING_IN_ARENA             583  // "No saving in the Arena"
#define LSTR_CLICK_TO_REMOVE_QUICKSPELL     584  // "Click here to remove your Quick Spell"
#define LSTR_ITEM_TOO_LAME                  585  // "Item is not of high enough quality"
#define LSTR_NOT_ENOUGH_SPELLPOINTS         586  // "Not enough spell points"
#define LSTR_ATTACK_BONUS                   587  // "Attack Bonus"
#define LSTR_ATTACK_DAMAGE                  588  // "Attack Damage"
#define LSTR_SHOOT_BONUS                    589  // "Shoot Bonus"
#define LSTR_SHOOT_DAMAGE                   590  // "Shoot Damage"
#define LSTR_CHARMED                        591  // "Charmed"
#define LSTR_SHRUNK                         592  // "Shrunk"
#define LSTR_SLOWED                         593  // "Slowed"
#define LSTR_WAND                           595  // "Wand"
#define LSTR_HISTORY                        602  // "History"
#define LSTR_PAY_FINE                       603  // "Pay Fine"
#define LSTR_BOUNTY_HUNT                    604  // "Bounty Hunt"
#define LSTR_CURRENT_FINE                   605  // "Current Fine"
#define LSTR_PAY                            606  // "Pay"
#define LSTR_ENSLAVED                       607  // "Enslaved"
#define LSTR_BERSERK                        608  // "Berserk"
#define LSTR_HOUR_OF_POWER                  609  // "Hour of Power"
#define LSTR_DAY_OF_PROTECTION              610  // "Day of Protection"
#define LSTR_PLAY_ARCOMAGE                  611  // "Play ArcoMage"
#define LSTR_FMT_SAVEGAME_CORRUPTED         612  // "Save game corrupted!  Code=%d"
#define LSTR_AUTOSAVE_MM7                   613  // "AutoSave.MM7"
#define LSTR_NEW_GAME                       614  // "New Game"
#define LSTR_SAVE_GAME                      615  // "Save Game"
#define LSTR_LOAD_GAME                      616  // "Load Game"
#define LSTR_OPTIONS                        617  // ""Sound, Keyboard, Game Options...""
#define LSTR_QUIT                           618  // "Quit"
#define LSTR_RETURN_TO_GAME                 619  // "Return to Game"
#define LSTR_RULES                          620  // "Rules"
#define LSTR_PLAY                           621  // "Play"
#define LSTR_VICTORY_CONDITIONS             622  // "Victory Conditions"
#define LSTR_BONUS_2                        623  // "Bonus"
#define LSTR_PHYSICAL                       624  // "Physical"
#define LSTR_IMMUNE                         625  // "Immune"
#define LSTR_RESISTANCES                    626  // "Resistances"
#define LSTR_RESISTANT                      627  // "Resistant"
#define LSTR_SPELL                          628  // "Spell"
#define LSTR_SPELLS                         629  // "Spells"
#define LSTR_UNKNOWN_VALUE                  630  // "?"
#define LSTR_EFFECTS                        631  // "Effects"
#define LSTR_FMT_SKILL_CANT_BE_LEARNED      632  // "This skill level can not be learned by the %s class."
#define LSTR_FMT_HAVE_TO_BE_PROMOTED        633  // "You have to be promoted to %s to learn this skill level."
#define LSTR_FMT_HAVE_TO_BE_PROMOTED_2      634  // "You have to be promoted to %s or %s to learn this skill level."
#define LSTR_FMT_S_STUNS_S                  635  // "%s stuns %s"
#define LSTR_FMT_S_PARALYZES_S              636  // "%s paralyzes %s"
#define LSTR_FMT_S_EVADES_DAMAGE            637  // "%s evades damage"
#define LSTR_HOSTILE_CREATURES_NEARBY       638  // "There are hostile creatures nearby!"
#define LSTR_A_TIE                          639  // "A tie!"
#define LSTR_YOU_WON                        640  // "You won!"
#define LSTR_YOU_LOST                       641  // "You lost!"
#define LSTR_SUMMONS_LIMIT_REACHED          648  // "This character can't summon any more monsters!"
#define LSTR_SUMMONED                       649  // "Summoned"
#define LSTR_CURRENT_HIT_POINTS             650  // "Current Hit Points"
#define LSTR_HARDENED                       651  // "Hardened"
#define LSTR_CANT_DO_UNDERWATER             652  // "You can not do that while you are underwater!"
#define LSTR_FOOD                           653  // "Food"
#define LSTR_FMT_JAR                        654  // "%s's Jar"
#define LSTR_FMT_JAR_2                      655  // "%s' Jar"
#define LSTR_GAME_SAVED                     656  // "Game Saved!"
#define LSTR_COLLECT_PRIZE                  658  // "Collect Prize"
#define LSTR_FMT_X_D_Y_D                    659  // "x: %d  y: %d"
#define LSTR_YOURE_DROWNING                 660  // "You're drowning!"
#define LSTR_ON_FIRE                        661  // "On fire!"
#define LSTR_INSTRUCTORS                    662  // "Instructors"
#define LSTR_FMT_IT_TAKES_D_DAY_TO_S        663  // "It will take %d day to cross to %s."
#define LSTR_ACCEPT_PARTY_HINT              664  // "Click here to accept this party and continue to the game."
#define LSTR_OK_BUTTON                      665  // "Ok Button"
#define LSTR_RESET_HINT                     666  // "Clears all party stats and skills."
#define LSTR_CLEAR_BUTTON                   667  // "Clear Button"
#define LSTR_SUBTRACT                       668  // "Subtract"
#define LSTR_SKILL_DECREASE_HINT            669  // ""Subtracts a point from the highlighted skill, returning it to the bonus pool""
#define LSTR_ADD                            670  // "Add"
#define LSTR_SKILL_INCREASE_HINT            671  // ""Adds a point from the highlighted skill, taking it from the bonus pool""
#define LSTR_ONE_YEAR_SENTENCE              672  // ""For your numerous crimes and evil deeds, you have been sentenced to one year in prison.""
#define LSTR_GOOD_ENDING                    675  // "Splendid job!  With the activation of the Gate, a thousand
                                                 // worlds lie at your feet.  Perhaps on one of them you will
                                                 // find the Ancients themselves, and return with the fruits
                                                 // their great civilization has to offer your world and your
                                                 // kingdom."
#define LSTR_EVIL_ENDING                    676  // "Brilliant!  The completion of the Heavenly Forge has
                                                 // provided enough Ancient weapons to crush all resistance to
                                                 // your plans.  Soon the world will bow to your every whim!
                                                 // Still, you can't help but wonder what was beyond the Gate
                                                 // the other side was trying so hard to build."

#define MM7_LOC_STRINGS 677

// OpenEnroth string IDs
#define LSTR_FMT_S_STOLE_D_ITEM             677  // "%s stole %s!"
#define LSTR_FMT_RECOVERY_TIME_D            678  // "Recovery time: %d"
#define LSTR_FMT_S_U_OUT_OF_U               679  // "%s: %lu out of %lu"
#define LSTR_NOBODY_IS_IN_CONDITION         680  // "Nobody is in a condition to do anything!"
#define LSTR_KEY_CONFLICT                   681  // "Please resolve all key conflicts!"
#define LSTR_RECOVERY_TIME_NA               682  // "Recovery time: N/A"
#define LSTR_WAND_ALREADY_CHARGED           683  // "Wand already charged!"
#define LSTR_ENERGY                         684  // "Energy"
#define LSTR_IMMOLATION_DAMAGE              685  // "Immolation deals %d damage to %d target(s)

#define MAX_LOC_STRINGS MM7_LOC_STRINGS + 9

class Localization {
 public:
    bool Initialize();

    const std::string &GetString(unsigned int index) const;

    template<class... Args>
    std::string FormatString(unsigned int index, Args &&... args) const {
        // TODO(captainurist): what if fmt throws?
        return fmt::sprintf(GetString(index), std::forward<Args>(args)...);
        // TODO(captainurist): there was also a call to sprintfex_internal after a call to vsprintf.
    }

    const std::string &GetDayName(unsigned int index) const {
        return this->day_names[index];
    }

    const std::string &GetMonthName(unsigned int index) const {
        return this->month_names[index];
    }

    const std::string &GetMoonPhaseName(unsigned int index) const {
        return this->moon_phase_names[index];
    }

    const std::string &GetSpellSchoolName(MagicSchool index) const {
        return this->spell_school_names[index];
    }

    const std::string &GetPartyBuffName(PartyBuff index) const {
        return this->party_buff_names[index];
    }

    const std::string &GetCharacterBuffName(CharacterBuff index) const {
        return this->character_buff_names[index];
    }

    const std::string &GetClassName(CharacterClass index) const {
        return this->class_names[index];
    }

    const std::string &GetClassDescription(CharacterClass index) const {
        return this->class_desciptions[index];
    }

    const std::string &GetAttirubteName(CharacterAttributeType index) const {
        return this->attribute_names[index];
    }

    const std::string &GetAttributeDescription(CharacterAttributeType index) const {
        return this->attribute_descriptions[index];
    }

    const std::string &GetSkillName(CharacterSkillType index) const {
        return this->skill_names[index];
    }

    const std::string &MasteryName(CharacterSkillMastery mastery) const {
        switch (mastery) {
        case CHARACTER_SKILL_MASTERY_NOVICE: return GetString(LSTR_NORMAL);
        case CHARACTER_SKILL_MASTERY_EXPERT: return GetString(LSTR_EXPERT);
        case CHARACTER_SKILL_MASTERY_MASTER: return GetString(LSTR_MASTER);
        case CHARACTER_SKILL_MASTERY_GRANDMASTER: return GetString(LSTR_GRAND);
        default:
            assert(false);
            return dummy_string;
        }
    }

    const std::string &MasteryNameLong(CharacterSkillMastery mastery) const {
        return mastery == CHARACTER_SKILL_MASTERY_GRANDMASTER ? GetString(LSTR_GRANDMASTER) : MasteryName(mastery);
    }

    const std::string &GetSkillDescription(CharacterSkillType index) const {
        return this->skill_descriptions[index];
    }

    const std::string &GetSkillDescription(CharacterSkillType index, CharacterSkillMastery mastery) const {
        switch(mastery) {
        case CHARACTER_SKILL_MASTERY_NOVICE: return GetSkillDescriptionNormal(index);
        case CHARACTER_SKILL_MASTERY_EXPERT: return GetSkillDescriptionExpert(index);
        case CHARACTER_SKILL_MASTERY_MASTER: return GetSkillDescriptionMaster(index);
        case CHARACTER_SKILL_MASTERY_GRANDMASTER: return GetSkillDescriptionGrand(index);
        default:
            assert(false);
            return dummy_string;
        }
    }

    const std::string &GetSkillDescriptionNormal(CharacterSkillType index) const {
        return this->skill_descriptions_normal[index];
    }

    const std::string &GetSkillDescriptionExpert(CharacterSkillType index) const {
        return this->skill_descriptions_expert[index];
    }

    const std::string &GetSkillDescriptionMaster(CharacterSkillType index) const {
        return this->skill_descriptions_master[index];
    }

    const std::string &GetSkillDescriptionGrand(CharacterSkillType index) const {
        return this->skill_descriptions_grand[index];
    }

    const std::string &GetCharacterConditionName(Condition index) const {
        return this->character_conditions[index];
    }

    const std::string &GetAmPm(unsigned int index) const {
        return this->GetString(472 + index);
    }

    const std::string &GetNpcProfessionName(NpcProfession prof) const {
        return this->npc_profession_names[prof];
    }

    const std::string &getHPDescription() const {
        return this->hp_description;
    }

    const std::string &getSPDescription() const {
        return this->sp_description;
    }

    const std::string &getArmourClassDescription() const {
        return this->armour_class_description;
    }

    const std::string &getCharacterConditionDescription() const {
        return this->character_condition_description;
    }

    const std::string &getFastSpellDescription() const {
        return this->fast_spell_description;
    }

    const std::string &getAgeDescription() const {
        return this->age_description;
    }

    const std::string &getLevelDescription() const {
        return this->level_description;
    }

    const std::string &getExpDescription() const {
        return this->exp_description;
    }

    const std::string &getMeleeAttackDescription() const {
        return this->melee_attack_description;
    }

    const std::string &getMeleeDamageDescription() const {
        return this->melee_damage_description;
    }

    const std::string &getRangedAttackDescription() const {
        return this->ranged_attack_description;
    }

    const std::string &getRangedDamageDescription() const {
        return this->ranged_damage_description;
    }

    const std::string &getFireResistanceDescription() const {
        return this->fire_res_description;
    }

    const std::string &getAirResistanceDescription() const {
        return this->air_res_description;
    }

    const std::string &getWaterResistanceDescription() const {
        return this->water_res_description;
    }

    const std::string &getEarthResistanceDescription() const {
        return this->earth_res_description;
    }

    const std::string &getMindResistanceDescription() const {
        return this->mind_res_description;
    }

    const std::string &getBodyResistanceDescription() const {
        return this->body_res_description;
    }

    const std::string &getSkillPointsDescription() const {
        return this->skill_points_description;
    }

 public:
    Localization() {}

 private:
    void InitializeMm6ItemCategories();

    void InitializeMonthNames();
    void InitializeDayNames();
    void InitializeMoonPhaseNames();

    void InitializeSpellSchoolNames();
    void InitializeSpellNames();

    void InitializeClassNames();
    void InitializeAttributeNames();
    void InitializeSkillNames();
    void InitializeCharacterConditionNames();

    void InitializeNpcProfessionNames();

 private:
    std::string localization_raw;
    std::vector<std::string> localization_strings;
    std::string class_desc_raw;
    std::string attribute_desc_raw;
    std::string skill_desc_raw;

    std::array<std::string, 14> mm6_item_categories;
    std::array<std::string, 12> month_names;
    std::array<std::string, 7> day_names;
    std::array<std::string, 5> moon_phase_names;
    IndexedArray<std::string, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> spell_school_names;
    IndexedArray<std::string, PARTY_BUFF_FIRST, PARTY_BUFF_LAST> party_buff_names;
    IndexedArray<std::string, CHARACTER_BUFF_FIRST, CHARACTER_BUFF_LAST> character_buff_names;
    IndexedArray<std::string, CLASS_FIRST, CLASS_LAST> class_names;
    IndexedArray<std::string, CLASS_FIRST, CLASS_LAST> class_desciptions;
    IndexedArray<std::string, CHARACTER_ATTRIBUTE_FIRST_STAT, CHARACTER_ATTRIBUTE_LAST_STAT> attribute_names;
    IndexedArray<std::string, CHARACTER_ATTRIBUTE_FIRST_STAT, CHARACTER_ATTRIBUTE_LAST_STAT> attribute_descriptions;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_names;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions_normal;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions_expert;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions_master;
    IndexedArray<std::string, CHARACTER_SKILL_INVALID, CHARACTER_SKILL_LAST_VISIBLE> skill_descriptions_grand;
    IndexedArray<std::string, CONDITION_FIRST, CONDITION_LAST> character_conditions;
    IndexedArray<std::string, NPC_PROFESSION_FIRST, NPC_PROFESSION_LAST> npc_profession_names;
    std::string hp_description;
    std::string sp_description;
    std::string armour_class_description;
    std::string character_condition_description;
    std::string fast_spell_description;
    std::string age_description;
    std::string level_description;
    std::string exp_description;
    std::string melee_attack_description;
    std::string melee_damage_description;
    std::string ranged_attack_description;
    std::string ranged_damage_description;
    std::string fire_res_description;
    std::string air_res_description;
    std::string water_res_description;
    std::string earth_res_description;
    std::string mind_res_description;
    std::string body_res_description;
    std::string skill_points_description;
    std::string dummy_string;
};

extern Localization *localization;
