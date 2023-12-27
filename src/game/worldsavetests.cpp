#include "worldsavetests.h"

void saveAndLoadWorldTest() {
    VoxelWorld world;
    world.userDataMap.insert_or_assign(ChunkCoord(1,0), std::unordered_map<BlockCoord, unsigned int, IntTupHash>());
    world.userDataMap.at(ChunkCoord(1,0)).insert_or_assign(BlockCoord(12,1,5), 1);
    world.userDataMap.at(ChunkCoord(1,0)).insert_or_assign(BlockCoord(13,1,5), 1);
    world.userDataMap.at(ChunkCoord(1,0)).insert_or_assign(BlockCoord(12,1,5), 1);
    world.userDataMap.insert_or_assign(ChunkCoord(2,0), std::unordered_map<BlockCoord, unsigned int, IntTupHash>());
    world.userDataMap.at(ChunkCoord(2,0)).insert_or_assign(BlockCoord(12,1,5), 1);
    world.userDataMap.at(ChunkCoord(2,0)).insert_or_assign(BlockCoord(13,1,5), 1);
    world.userDataMap.at(ChunkCoord(2,0)).insert_or_assign(BlockCoord(12,1,5), 1);
    world.seed = time(NULL);

    std::cout << "World saving has: \n";
    std::cout << "  Seed: " << world.seed << '\n';
    for(auto &[chunkcoord, map] : world.userDataMap) {
        std::cout << "  Chunk at: " << chunkcoord.x << "," << chunkcoord.z << '\n';
        for(auto &[blockcoord, id] : map) {
            std::cout << "      Has block: " << blockcoord.x << "," << blockcoord.y << "," << blockcoord.z << '\n';
        }
    }

    world.saveWorldToFile("world.file");

    world.loadWorldFromFile("world.file");

    std::cout << "World loaded has: \n";
    std::cout << "  Seed: " << world.seed << '\n';
    for(auto &[chunkcoord, map] : world.userDataMap) {
        std::cout << "  Chunk at: " << chunkcoord.x << "," << chunkcoord.z << '\n';
        for(auto &[blockcoord, id] : map) {
            std::cout << "      Has block: " << blockcoord.x << "," << blockcoord.y << "," << blockcoord.z << '\n';
        }
    }
}