#ifndef SIGNWORDS_H

#define SIGNWORDS_H

#include <unordered_map>
#include "../util/chunkcoord.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <ostream>
#include <iostream>

extern std::unordered_map<BlockCoord, std::string, IntTupHash> signWords;
extern std::string signWordsString;
extern std::string SIGN_BUFFER;
void saveSignWordsToFile();

void loadSignWordsFromFile();
void loadSignWordsFromString(std::string& signString);
#endif