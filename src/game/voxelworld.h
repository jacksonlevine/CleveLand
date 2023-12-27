#include <unordered_map>
#include "../util/chunkcoord.h"

class VoxelWorld {

    unsigned int seed;

    std::unordered_map<
        ChunkCoord, 
        std::unordered_map<
            BlockCoord, 
            unsigned int, 
            IntTupHash>, 
        IntTupHash>        userDataMap;
    
    void saveWorldToFile(const char *path);
    void loadWorldFromFile(const char *path);
};