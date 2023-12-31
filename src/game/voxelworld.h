#ifndef VOXELWORLD_H
#define VOXELWORLD_H

#include <unordered_map>
#include "../util/chunkcoord.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "../util/perlin.h"
#include <filesystem>
#include <string>
#include <thread>
#include <mutex>
#include "blockchunk.h"
#include "nuggo.h"
#include "../util/textureface.h"


class VoxelWorld {
public:
    unsigned int seed;

    inline static int chunkWidth = 16;
    inline static int chunkHeight = 64;

    inline static bool runChunkThread = false;

    std::unordered_map<
        ChunkCoord,
        BlockChunk,
        IntTupHash
    >                   takenCareOfChunkSpots;

    glm::vec3 cameraPosition;
    glm::vec3 cameraDirection;

    Perlin perlin;

    std::unordered_map<
        ChunkCoord, 
        std::unordered_map<
            BlockCoord, 
            unsigned int, 
            IntTupHash>, 
        IntTupHash>        userDataMap;


    std::vector<BlockChunk> chunks;
    std::vector<Nuggo> nuggoPool;


    void populateChunksAndNuggos(entt::registry &registry);

    void rebuildChunk(BlockChunk chunk, ChunkCoord newPosition);

    void chunkUpdateThreadFunction();

    std::vector<unsigned int> nuggosToRebuild;


    inline static bool shouldTryReload = false;


    std::thread chunkUpdateThread;
    std::mutex meshQueueMutex;

    float noiseFunction(int x, int y, int z);
    unsigned int blockAt(BlockCoord coord);

    void runStep(float deltaTime);
    
    bool saveExists(const char* path);
    void saveWorldToFile(const char *path) noexcept(false);
    void loadWorldFromFile(const char *path) noexcept(false);
};
#endif