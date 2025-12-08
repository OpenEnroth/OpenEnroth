#pragma once

#include "MagicEnums.h"
#include "MagicEnumFunctions.h"

class Blob;

MagicFileFormat magic(const Blob &data);
