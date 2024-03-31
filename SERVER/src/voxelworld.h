#include <unordered_map>
#include "chunkcoord.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <glm/glm.hpp>

extern std::string worldString;

class VoxelWorld {


    public:


    inline static unsigned int seed = 0;
    inline static int worldGenVersion = 2;

    inline static float timeOfDay = 0.0f;
    inline static float dayLength = 900.0f;

    inline static int chunkWidth = 16;
    inline static int chunkHeight = 128;

    std::unordered_map<
        ChunkCoord, 
        std::unordered_map<
            BlockCoord, 
            uint32_t, 
            IntTupHash>, 
        IntTupHash>        userDataMap;

    bool saveExists(const char* path);
    void _saveWorldToFile(const char *path) noexcept(false);
    void _loadWorldFromFile(const char *path) noexcept(false);

    void setBlock(BlockCoord coord, uint32_t block);

    void saveGame(const char* path);
    void loadOrCreateSaveGame(const char* path);
};