#pragma once

#include "Engine/Evt/EvtEnums.h"

class Character;

/**
 * The functions below index tables with the values they are given, so a script value has to pass this first.
 *
 * @param opcode                        `EVENT_Compare`, `EVENT_OnCanShowDialogItemCmp`, `EVENT_Set`, `EVENT_Add`
 *                                      or `EVENT_Subtract`.
 * @param var                           Variable the command works on.
 * @param value                         Value from the script.
 * @return                              Whether `value` is in range for `var` under `opcode`.
 */
[[nodiscard]] bool isEvtVariableValueValid(EvtOpcode opcode, EvtVariable var, int value);

void setEvtVariable(Character &character, EvtVariable var, signed int a3);
void addEvtVariable(Character &character, EvtVariable var, signed int val);

/**
 * @return                          False if the subtraction could not be performed, e.g. the party doesn't have
 *                                  enough gold. A script that hits this is aborted.
 */
[[nodiscard]] bool subtractEvtVariable(Character &character, EvtVariable VarNum, signed int pValue);

bool compareEvtVariable(Character &character, EvtVariable VarNum, signed int pValue);
