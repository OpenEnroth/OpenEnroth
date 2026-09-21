#pragma once

#include "Engine/Evt/EvtEnums.h"

class Character;

void setEvtVariable(Character &character, EvtVariable var, signed int a3);
void addEvtVariable(Character &character, EvtVariable var, signed int val);

/**
 * @return                          False if the subtraction could not be performed, e.g. the party doesn't have
 *                                  enough gold. A script that hits this is aborted.
 */
[[nodiscard]] bool subtractEvtVariable(Character &character, EvtVariable VarNum, signed int pValue);

bool compareEvtVariable(Character &character, EvtVariable VarNum, signed int pValue);
