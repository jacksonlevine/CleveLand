#ifndef SIGNWORDS_H

#define SIGNWORDS_H

#include <unordered_map>
#include "../util/chunkcoord.h"
#include <string>

extern std::unordered_map<BlockCoord, std::string, IntTupHash> signWords;


#endif