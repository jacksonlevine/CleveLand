#ifndef VOXELWORLD_H
#define VOXELWORLD_H

class VoxelWorld;

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
#include "../game/specialblocks/post.h"
#include <atomic>
#include "../util/lightinfo.h"
#include <GLFW/glfw3.h>
#include <stack>
#include "specialblocks/chest.h"
#include "specialblocks/ladder.h"
#include "specialblocks/torch.h"
#include "specialblocks/sign.h"
#include "../soundfxsystem.h"


class VoxelWorld {
public:

    std::mutex MASTERLOCK;

    std::mutex udmMutex;


    inline static unsigned int seed = 0;

    inline static std::function<float(int, int, int)>* currentNoiseFunction = 0;


    inline static int worldGenVersion = 1;

    inline static int chunkWidth = 16;
    inline static int chunkHeight = 128;

    inline static std::atomic<bool> runChunkThread = false;

     inline static glm::ivec3 worldOffset = glm::ivec3(0,0,0);
    void getOffsetFromSeed();

    std::function<void(int,int,int,uint32_t)> *multiplayerBlockSetFunc;

    std::unordered_map<
        ChunkCoord,
        BlockChunk*,
        IntTupHash
    >                   takenCareOfChunkSpots;
    std::mutex takenCareOfChunkMutex;

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

    std::mutex nudmMutex;


    std::unordered_map<
            BlockCoord,
            LightSegment,
            IntTupHash
    >                       lightMap;

    std::unordered_map<
            BlockCoord,
            LightSegment,
            IntTupHash
    >                       lightMapAmbient;

    // void depropogateLightOrigin(BlockCoord spot, BlockCoord origin, std::set<BlockChunk*> *imp, std::unordered_map<
    //         BlockCoord,
    //         LightSegment,
    //         IntTupHash
    // >& lightMap);
    void depropogateLightOriginIteratively(BlockCoord origin, std::set<BlockChunk*> *imp, std::unordered_map<BlockCoord,LightSegment,IntTupHash>& lightMap);
    // void propogateLightOrigin(BlockCoord spot, BlockCoord origin, int value, std::set<BlockChunk*> *imp, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo, std::unordered_map<
    //         BlockCoord,
    //         LightSegment,
    //         IntTupHash
    // > &lightMap);
    void propogateLightOriginIteratively(BlockCoord spot, BlockCoord origin, int value, std::set<BlockChunk*> *imp, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo, std::unordered_map<
            BlockCoord,
            LightSegment,
            IntTupHash
    > &lightMap);

    void lightPassOnChunk(ChunkCoord chunkcoord, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo);



    std::vector<BlockChunk> chunks;
    std::vector<GeometryStore> geometryStorePool;

    void generateChunk(ChunkCoord chunkcoord, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo);


    void populateChunksAndGeometryStores(entt::registry &registry, int viewDistance);

    void rebuildChunk(BlockChunk *chunk, ChunkCoord newPosition, bool immediateInPlace, bool light, bool user = false);

    void chunkUpdateThreadFunctionUser(int loadRadius);

    void chunkUpdateThreadFunction(int loadRadius);

    std::vector<BlockChunk*> getPreferredChunkPtrList(int loadRadius, ChunkCoord& cameraChunkPos);

    boost::lockfree::queue<int, boost::lockfree::capacity<2304>> geometryStoreQueue;
    boost::lockfree::queue<int, boost::lockfree::capacity<2304>> highPriorityGeometryStoreQueue;
    boost::lockfree::queue<int, boost::lockfree::capacity<2304>> userGeometryStoreQueue;

    boost::lockfree::queue<BlockChunk*, boost::lockfree::capacity<2304>> deferredChunkQueue;
    boost::lockfree::queue<BlockChunk*, boost::lockfree::capacity<2304>> userPowerQueue;

    boost::lockfree::queue<BlockChunk*, boost::lockfree::capacity<2304>> lightUpdateQueue;

    std::unordered_map<
        ChunkCoord,
        bool,
        IntTupHash
    >                   hasHadInitialLightPass;
    
    std::mutex hashadlightMutex;

    std::mutex lightMapMutex;
    
    inline static bool shouldTryReload = false;


    std::thread chunkUpdateThread;
    std::thread userChunkUpdateThread;

    static float noiseFunction(int x, int y, int z);


    static float noiseFunction2(int x, int y, int z);

    
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

    void locallySetBlock(BlockCoord coord, uint32_t block);
    void setBlock(BlockCoord coord, uint32_t block, bool updateMultiplayer = true);
    void setBlockAndQueueRerender(BlockCoord coord, uint32_t block);

    void checkAboveHeadThreadFunction();
    
    inline static int initialLoadProgress = 0;

    inline static std::atomic<bool> stillRunningThread = false;

    inline static int waterLevel = 40;

    inline static float timeChunkMeshing = 0.0f;
    inline static int numberOfSamples = 0;

    uint32_t blockAtMemo(BlockCoord coord, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo);
};
#endif