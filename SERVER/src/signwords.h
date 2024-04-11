#ifndef SIGNWORDS_H

#define SIGNWORDS_H

#include <unordered_map>
#include "chunkcoord.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <ostream>
#include <iostream>

extern std::unordered_map<BlockCoord, std::string, IntTupHash> signWords;
extern std::string signWordsString;
void saveSignWordsToFile();

void loadSignWordsFromFile();

#endif