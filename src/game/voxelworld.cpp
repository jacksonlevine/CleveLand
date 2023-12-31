#include "voxelworld.h"


void VoxelWorld::chunkUpdateThreadFunction() {
    

    static glm::vec3 lastCamPosDivided;
    static bool first = true;

    static int loadRadius = 7;
    while(runChunkThread) {
        glm::vec3 currCamPosDivided = cameraPosition/10.0f;
        if(currCamPosDivided != lastCamPosDivided || first) {
            lastCamPosDivided = currCamPosDivided;
            first = false;

            BlockCoord cameraBlockPos(std::round(cameraPosition.x), std::round(cameraPosition.y), std::round(cameraPosition.z));
            ChunkCoord cameraChunkPos(std::floor(static_cast<float>(cameraBlockPos.x)/chunkWidth), std::floor(static_cast<float>(cameraBlockPos.z)/chunkWidth));
            ChunkCoord cameraChunkPosAdjustedWithDirection(
                static_cast<int>(std::floor(static_cast<float>(cameraBlockPos.x)/chunkWidth) + (cameraDirection.x * 4)), 
                static_cast<int>(std::floor(static_cast<float>(cameraBlockPos.z)/chunkWidth) + (cameraDirection.z * 4))
                );

            std::sort(chunks.begin(), chunks.end(), [cameraChunkPosAdjustedWithDirection](BlockChunk& a, BlockChunk& b){
                int adist = 
                std::abs(a.position.x - cameraChunkPosAdjustedWithDirection.x) +
                std::abs(a.position.z - cameraChunkPosAdjustedWithDirection.z);

                int bdist = 
                std::abs(b.position.x - cameraChunkPosAdjustedWithDirection.x) +
                std::abs(b.position.z - cameraChunkPosAdjustedWithDirection.z);

                return adist > bdist;
            });

            meshQueueMutex.lock();


            int takenChunkIndex = 0;
            for(int x = cameraChunkPos.x - loadRadius; x < cameraChunkPos.x + loadRadius; ++x) {
                for(int z = cameraChunkPos.z - loadRadius; z < cameraChunkPos.z + loadRadius; ++z) {

                    ChunkCoord thisChunkCoord(x,z);
                    if(takenCareOfChunkSpots.find(thisChunkCoord) == takenCareOfChunkSpots.end()) {
                        
                        rebuildChunk(chunks[takenChunkIndex], thisChunkCoord);

                        takenChunkIndex++;
                    }

                }
            }

            meshQueueMutex.unlock();
        }
    }

}

void VoxelWorld::populateChunksAndNuggos(entt::registry &registry) {
    for(int i = 0; i < 20; ++i) {
        for(int j = 0; j < 20; ++j) {

            Nuggo nuggo;
            nuggo.me = registry.create();

            BlockChunk chunk;
            chunk.nuggoPoolIndex = nuggoPool.size();

            chunks.push_back(chunk);

            nuggoPool.push_back(nuggo);


        }
    }
}


void VoxelWorld::rebuildChunk(BlockChunk &chunk, ChunkCoord newPosition) {

    if(takenCareOfChunkSpots.find(chunk.position) != takenCareOfChunkSpots.end()) {
        takenCareOfChunkSpots.erase(chunk.position);
    }
    chunk.position = newPosition;
    
    int startX = chunk.position.x * chunkWidth;
    int startZ = chunk.position.z * chunkWidth;
    int startY = 0;

    //left right forward backward up down
    static std::vector<BlockCoord> neighbors = {
        BlockCoord(-1, 0, 0),
        BlockCoord(1, 0, 0),
        BlockCoord(0, 0, 1),
        BlockCoord(0, 0, -1),
        BlockCoord(0, 1, 0),
        BlockCoord(0, -1, 0),
    };

    static std::vector<std::vector<float>> faces = {
           
    {
        -0.5f, -0.5f, 0.5f, 10.0f,
        -0.5f, -0.5f, -0.5f, 10.0f,
        -0.5f, 0.5f, -0.5f, 10.0f,

        -0.5f, 0.5f, -0.5f, 10.0f,
        -0.5f, 0.5f, 0.5f, 10.0f,
        -0.5f, -0.5f, 0.5f, 10.0f
    },
    {
        0.5f, -0.5f, -0.5f, 10.0f,
        0.5f, -0.5f, 0.5f, 10.0f,
        0.5f, 0.5f, 0.5f, 10.0f,

        0.5f, 0.5f, 0.5f, 10.0f,
        0.5f, 0.5f, -0.5f, 10.0f,
        0.5f, -0.5f, -0.5f, 10.0f
    },
    {
        0.5f, -0.5f, 0.5f, 14.0f,
        -0.5f, -0.5f, 0.5f, 14.0f,
        -0.5f, 0.5f, 0.5f, 14.0f,

        -0.5f, 0.5f, 0.5f, 14.0f,
        0.5f, 0.5f, 0.5f, 14.0f,
        0.5f, -0.5f, 0.5f, 14.0f
    },
    {
        -0.5f, -0.5f, -0.5f,  14.0f,
        0.5f, -0.5f, -0.5f, 14.0f,
        0.5f, 0.5f, -0.5f, 14.0f,

        0.5f, 0.5f, -0.5f, 14.0f,
        -0.5f, 0.5f, -0.5f, 14.0f,
        -0.5f, -0.5f, -0.5f, 14.0f
    },
     {
        -0.5f, 0.5f, -0.5f, 16.0f,
        0.5f, 0.5f, -0.5f, 16.0f,
        0.5f, 0.5f, 0.5f, 16.0f,

        0.5f, 0.5f, 0.5f, 16.0f,
        -0.5f, 0.5f, 0.5f, 16.0f,
        -0.5f, 0.5f, -0.5f, 16.0f,
    },
    {
        0.5f, -0.5f, -0.5f, 7.0f,
        -0.5f, -0.5f, -0.5f, 7.0f,
        -0.5f, -0.5f, 0.5f, 7.0f,

        -0.5f, -0.5f, 0.5f, 7.0f,
        0.5f, -0.5f, 0.5f, 7.0f,
        0.5f, -0.5f, -0.5f,  7.0f
    }
    };

    TextureFace tex(0,0);

    std::vector<float> verts;
    std::vector<float> uvs;

    for(int x = startX; x < startX + chunkWidth; ++x) {
        for(int z = startZ; z < startZ + chunkWidth; ++z) {
            for(int y = startY; y < startY + chunkHeight; ++y) {
                BlockCoord coord(x,y,z);
                int neighborIndex = 0;
                if(blockAt(coord) != 0) {
                    for(BlockCoord &neigh : neighbors) {
                        if(blockAt(coord + neigh) == 0) {
                            verts.insert(verts.end(), {
                                faces[neighborIndex][0]+coord.x,faces[neighborIndex][1]+coord.y, faces[neighborIndex][2]+coord.z, faces[neighborIndex][3],
                                faces[neighborIndex][4]+coord.x,faces[neighborIndex][5]+coord.y, faces[neighborIndex][6]+coord.z, faces[neighborIndex][7],
                                faces[neighborIndex][8]+coord.x,faces[neighborIndex][9]+coord.y, faces[neighborIndex][10]+coord.z, faces[neighborIndex][11],

                                faces[neighborIndex][12]+coord.x,faces[neighborIndex][13]+coord.y, faces[neighborIndex][14]+coord.z, faces[neighborIndex][15],
                                faces[neighborIndex][16]+coord.x,faces[neighborIndex][17]+coord.y, faces[neighborIndex][18]+coord.z, faces[neighborIndex][19],
                                faces[neighborIndex][20]+coord.x,faces[neighborIndex][21]+coord.y, faces[neighborIndex][22]+coord.z, faces[neighborIndex][23],
                            });
                            uvs.insert(uvs.end() , {
                                tex.bl.x, tex.bl.y,
                                tex.br.x, tex.br.y,
                                tex.tr.x, tex.tr.y,

                                tex.tr.x, tex.tr.y,
                                tex.tl.x, tex.tl.y,
                                tex.bl.x, tex.bl.y
                            });
                        }
                        neighborIndex++;
                    }
                }
            }
        }
    }

    nuggoPool[chunk.nuggoPoolIndex].verts = verts;
    nuggoPool[chunk.nuggoPoolIndex].uvs = uvs;

    bool found = false;
    for(int i : nuggosToRebuild) {
        if(i == chunk.nuggoPoolIndex) {
            found = true;
        }
    }

    if(!found) {
        nuggosToRebuild.push_back(chunk.nuggoPoolIndex);
    }

    if(takenCareOfChunkSpots.find(chunk.position) == takenCareOfChunkSpots.end()) {
        takenCareOfChunkSpots.insert_or_assign(chunk.position, 0);
    }

}

















unsigned int VoxelWorld::blockAt(BlockCoord coord) {
    ChunkCoord chunkcoord(
        static_cast<int>(std::floor(static_cast<float>(coord.x)/chunkWidth)), 
        static_cast<int>(std::floor(static_cast<float>(coord.z)/chunkWidth))
    );
    auto chunkit = userDataMap.find(chunkcoord);
    if(chunkit != userDataMap.end()) {
        auto blockit = chunkit->second.find(coord);
        if(blockit != chunkit->second.end()) {
            return blockit->second;
        }
    }
    if(noiseFunction(coord.x, coord.y, coord.z) > 10) {
        return 1; //replace this with a "getWorldBlock" function later
    }
    return 0;
}

float VoxelWorld::noiseFunction(int x, int y, int z) {
    return 
    std::max(0.0f, (
        20.0f + static_cast<float>(perlin.noise((static_cast<float>(x))/15.35f, (static_cast<float>(y+(seed/100)))/15.35f, (static_cast<float>(z))/15.35f)) * 10.0f
    ) - std::max(((float)y/3.0f), 0.0f));
}

bool VoxelWorld::saveExists(const char* path) {
    return std::filesystem::exists(path);
}

void VoxelWorld::saveWorldToFile(const char *path) {
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());

    std::ofstream outputFile(path, std::ios::trunc);
    if(outputFile.is_open()) {
        outputFile << seed << '\n';
        for(auto &[chunkcoord, map] : userDataMap) {
            outputFile << chunkcoord.x << " " << chunkcoord.z << "\n";
            for(auto &[blockcoord, blockid] : map) {
                outputFile << "% " << blockcoord.x << " " << blockcoord.y << " " << blockcoord.z << " " <<
                blockid << "\n";
            }
        }
        outputFile.close();
    } else {
        throw std::exception("Failed to save world.");
    }
}

void VoxelWorld::loadWorldFromFile(const char *path) {
    userDataMap.clear();
    std::ifstream file(path);
    if(file.is_open()) {
        std::string line;
        ChunkCoord currentChunkCoord;
        int lineNumber = 0;
        while(std::getline(file, line)) {
            if(lineNumber == 0) {
                seed = static_cast<unsigned int>(std::stoi(line));
            } else {
                std::istringstream linestream(line);
                if(line.front() != '%') {
                    std::string word;
                    int localIndex = 0;
                    ChunkCoord thisChunkCoord;
                    while(linestream >> word) {
                        if(localIndex == 0) {
                            thisChunkCoord.x = std::stoi(word);
                        }
                        if(localIndex == 1) {
                            thisChunkCoord.z = std::stoi(word);
                        }
                        localIndex++;
                    }
                    currentChunkCoord = thisChunkCoord;
                    std::unordered_map<BlockCoord, unsigned int, IntTupHash> thisBlockMap;
                    userDataMap.insert_or_assign(currentChunkCoord, thisBlockMap);
                } else {
                    std::string word;
                    int localIndex = 0;
                    BlockCoord thisBlockCoord;
                    unsigned int thisID;
                    while(linestream >> word) {
                        if(localIndex == 1) {
                            thisBlockCoord.x = std::stoi(word);
                        }
                        if(localIndex == 2) {
                            thisBlockCoord.y = std::stoi(word);
                        }
                        if(localIndex == 3) {
                            thisBlockCoord.z = std::stoi(word);
                        }
                        if(localIndex == 4) {
                            thisID = static_cast<unsigned int>(std::stoi(word));
                        }
                        localIndex++;
                    }
                    userDataMap.at(currentChunkCoord).insert_or_assign(thisBlockCoord, thisID);
                }
            }
            lineNumber++;
        }
    } else {
        throw std::exception("Could not open world file.");
    }
}