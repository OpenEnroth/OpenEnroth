#include "Engine/Events.h"
#include "Engine/Events2D.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"

// TODO(Nik-RE-dev): move remaining globals

std::array<unsigned int, 500> pLevelStrOffsets;
unsigned int uLevelStrNumStrings;
unsigned int uLevelStrFileSize;
std::array<char, 9216> pLevelStr;

_2devent p2DEvents[525];

MapEventVariables mapEventVariables;

int savedEventID;
int savedEventStep;
struct LevelDecoration *savedDecoration;

