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
    inline static unsigned int seed = 0;

    inline static std::function<float(int, int, int)>* currentNoiseFunction = 0;


    inline static int worldGenVersion = 1;

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

    inline static Perlin perlin;

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

    void rebuildChunk(BlockChunk *chunk, ChunkCoord newPosition, bool immediateInPlace, bool light);

    void chunkUpdateThreadFunction(int loadRadius);

    std::vector<BlockChunk*> getPreferredChunkPtrList(int loadRadius, ChunkCoord& cameraChunkPos);

    boost::lockfree::queue<int, boost::lockfree::capacity<2304>> geometryStoreQueue;
    boost::lockfree::queue<int, boost::lockfree::capacity<2304>> highPriorityGeometryStoreQueue;


    boost::lockfree::queue<BlockChunk*, boost::lockfree::capacity<2304>> deferredChunkQueue;

    boost::lockfree::queue<BlockChunk*, boost::lockfree::capacity<2304>> lightUpdateQueue;

    std::unordered_map<
        ChunkCoord,
        bool,
        IntTupHash
    >                   hasHadInitialLightPass;

    
    inline static bool shouldTryReload = false;


    std::thread chunkUpdateThread;


    inline static float noiseFunction(int x, int y, int z) {
        return 
        std::max(0.0f, (
            20.0f + static_cast<float>(perlin.noise((static_cast<float>(x))/20.35f, (static_cast<float>(y+(seed/100)))/20.35f, (static_cast<float>(z))/20.35f)) * 5.0f
        ) - std::max(((float)y/2.0f) + static_cast<float>(perlin.noise(x/65.0f, z/65.0f)) * 10.0f, 0.0f));
    }


    inline static float noiseFunction2(int x, int y, int z) {
        return 
        std::max(0.0f, (
            20.0f + static_cast<float>(perlin.noise((static_cast<float>(x))/105.35f, (static_cast<float>(y))/105.35f, (static_cast<float>(z))/105.35f)) * 5.0f
        ) - std::max(((float)y/4.0f) /*+ static_cast<float>(perlin.noise(x/15.0f, z/15.0f)) * 2.0f*/, 0.0f))
        
        - static_cast<float>(perlin.noise((static_cast<float>(x))/25.35f, (static_cast<float>(y))/25.35f, (static_cast<float>(z))/25.35f)) * 10.0f;
    }

    
    inline static std::vector<std::function<float(int, int, int)>> worldGenFunctions = {
        [](int x, int y, int z){
            //placeholder
            return 0.0f;
        },
        [](int x, int y, int z){
            return VoxelWorld::noiseFunction(x,y,z);
        },
        [](int x, int y, int z){
            return VoxelWorld::noiseFunction2(x,y,z);
        }
    };

    uint32_t blockAt(BlockCoord coord);

    void runStep(float deltaTime);
    
    bool saveExists(const char* path);
    void saveWorldToFile(const char *path) noexcept(false);
    void loadWorldFromFile(const char *path) noexcept(false);
    int checkVersionOfSave(const char *path);
    void deleteFolder(std::string path);
    
    inline static int initialLoadProgress = 0;

    inline static std::atomic<bool> stillRunningThread = false;
};
#endif