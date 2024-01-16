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
#include "geometrystore.h"
#include "../util/textureface.h"
#include <boost/lockfree/queue.hpp>
#include "../util/random.h"
#include "../util/blockinfo.h"
#include "../game/specialblocks/door.h"
#include <atomic>
#include "../util/lightinfo.h"

class VoxelWorld {
public:
    unsigned int seed;

    inline static int chunkWidth = 16;
    inline static int chunkHeight = 128;

    inline static bool runChunkThread = false;

    std::unordered_map<
        ChunkCoord,
        BlockChunk*,
        IntTupHash
    >                   takenCareOfChunkSpots;

    glm::vec3 cameraPosition;
    glm::vec3 cameraDirection;

    Perlin perlin;

    std::unordered_map<
        ChunkCoord, 
        std::unordered_map<
            BlockCoord, 
            uint32_t, 
            IntTupHash>, 
        IntTupHash>        userDataMap;

    std::unordered_map<
            BlockCoord, 
            uint32_t, 
            IntTupHash>
                           nonUserDataMap;


    std::unordered_map<
            BlockCoord,
            LightSegment,
            IntTupHash
    >                       lightMap;

    void depropogateLightOrigin(BlockCoord spot, BlockCoord origin, std::set<BlockChunk*> *imp);
    void propogateLightOrigin(BlockCoord spot, BlockCoord origin, int value, std::set<BlockChunk*> *imp);
    void lightPassOnChunk(ChunkCoord chunkcoord);



    std::vector<BlockChunk> chunks;
    std::vector<GeometryStore> geometryStorePool;

    void generateChunk(ChunkCoord chunkcoord);


    void populateChunksAndGeometryStores(entt::registry &registry, int viewDistance);

    void rebuildChunk(BlockChunk *chunk, ChunkCoord newPosition, bool immediateInPlace);

    void chunkUpdateThreadFunction(int loadRadius);

    std::vector<BlockChunk*> getPreferredChunkPtrList(int loadRadius, ChunkCoord& cameraChunkPos);

    boost::lockfree::queue<int> geometryStoreQueue;
    boost::lockfree::queue<int> highPriorityGeometryStoreQueue;


    boost::lockfree::queue<BlockChunk*> deferredChunkQueue;

    
    inline static bool shouldTryReload = false;


    std::thread chunkUpdateThread;


    float noiseFunction(int x, int y, int z);
    uint32_t blockAt(BlockCoord coord);

    void runStep(float deltaTime);
    
    bool saveExists(const char* path);
    void saveWorldToFile(const char *path) noexcept(false);
    void loadWorldFromFile(const char *path) noexcept(false);
    
    inline static int initialLoadProgress = 0;

    inline static std::atomic<bool> stillRunningThread = false;
};
#endif