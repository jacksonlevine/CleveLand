#ifndef VOXELWORLD_H
#define VOXELWORLD_H

#include <unordered_map>
#include "../util/chunkcoord.h"
#include <iostream>
#include <fstream>
#include <sstream>

class VoxelWorld {
public:
    unsigned int seed;

    std::unordered_map<
        ChunkCoord, 
        std::unordered_map<
            BlockCoord, 
            unsigned int, 
            IntTupHash>, 
        IntTupHash>        userDataMap;
    
    void saveWorldToFile(const char *path) noexcept(false);
    void loadWorldFromFile(const char *path) noexcept(false);
};
#endif