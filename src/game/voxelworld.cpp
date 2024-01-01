#include "voxelworld.h"

void VoxelWorld::runStep(float deltaTime) {
    static float timer = 0.0f;

    if(timer > 5.0f) {
        shouldTryReload = true;

        timer = 0.0f;
    } else {
        timer += deltaTime;
    }
}


void VoxelWorld::chunkUpdateThreadFunction() {
    
    std::cout << "Thread started \n";
    glm::vec3 lastCamPosDivided;
    bool first = true;

    static int loadRadius = 7;
    while(runChunkThread) {
        glm::vec3 currCamPosDivided = cameraPosition/10.0f;
        if(currCamPosDivided != lastCamPosDivided || first || shouldTryReload) {
            lastCamPosDivided = currCamPosDivided;

            if(first) {
                std::cout << "First \n";
            }

            shouldTryReload = false;

            BlockCoord cameraBlockPos(std::round(cameraPosition.x), std::round(cameraPosition.y), std::round(cameraPosition.z));
            ChunkCoord cameraChunkPos(std::floor(static_cast<float>(cameraBlockPos.x)/chunkWidth), std::floor(static_cast<float>(cameraBlockPos.z)/chunkWidth));
            ChunkCoord cameraChunkPosAdjustedWithDirection(
                static_cast<int>(std::floor(static_cast<float>(cameraBlockPos.x)/chunkWidth) + (cameraDirection.x * 4)), 
                static_cast<int>(std::floor(static_cast<float>(cameraBlockPos.z)/chunkWidth) + (cameraDirection.z * 4))
                );

            // std::sort(chunks.begin(), chunks.end(), [cameraChunkPosAdjustedWithDirection](BlockChunk& a, BlockChunk& b){
            //     int adist = 
            //     std::abs(a.position.x - cameraChunkPosAdjustedWithDirection.x) +
            //     std::abs(a.position.z - cameraChunkPosAdjustedWithDirection.z);

            //     int bdist = 
            //     std::abs(b.position.x - cameraChunkPosAdjustedWithDirection.x) +
            //     std::abs(b.position.z - cameraChunkPosAdjustedWithDirection.z);

            //     return adist > bdist;
            // });

            //MAYBE SWITCH BACK TO NON-ADJUSTED CAMERA CHUNK POS HERE VVV

           std::vector<BlockChunk*> sortedChunkPtrs;

            // Fill the vector with pointers to the elements of chunks
            for (auto& chunk : chunks) {
                sortedChunkPtrs.push_back(&chunk);
            }

            std::partition(sortedChunkPtrs.begin(), sortedChunkPtrs.end(), [cameraChunkPosAdjustedWithDirection](BlockChunk *chunk) {
                
                if(!chunk->used) {
                    return true;
                }

                int dist = 
                std::abs(chunk->position.x - cameraChunkPosAdjustedWithDirection.x) +
                std::abs(chunk->position.z - cameraChunkPosAdjustedWithDirection.z);

                return dist >= loadRadius;
            });

            if(meshQueueMutex.try_lock()) {


                int takenChunkIndex = 0;
                for(int x = cameraChunkPos.x - loadRadius; x < cameraChunkPos.x + loadRadius; ++x) {
                    for(int z = cameraChunkPos.z - loadRadius; z < cameraChunkPos.z + loadRadius; ++z) {

                        ChunkCoord thisChunkCoord(x,z);
                        if(takenCareOfChunkSpots.find(thisChunkCoord) == takenCareOfChunkSpots.end()) {

                            rebuildChunk(*sortedChunkPtrs[takenChunkIndex], thisChunkCoord);

                            takenChunkIndex++;
                        }

                    }
                }

                meshQueueMutex.unlock();
            }
            first = false;
        }
    }

    std::cout << "Thread ended \n";
}

void VoxelWorld::populateChunksAndGeometryStores(entt::registry &registry) {
    for(int i = 0; i < 20; ++i) {
        for(int j = 0; j < 20; ++j) {

            GeometryStore geometryStore;
            geometryStore.me = registry.create();

            BlockChunk chunk;
            chunk.geometryStorePoolIndex = geometryStorePool.size();
            chunk.used = false;
            chunks.push_back(chunk);

            geometryStorePool.push_back(geometryStore);


        }
    }
}


void VoxelWorld::rebuildChunk(BlockChunk &chunk, ChunkCoord newPosition) {

    if(takenCareOfChunkSpots.find(chunk.position) != takenCareOfChunkSpots.end()) {
        takenCareOfChunkSpots.erase(chunk.position);
    }
    chunk.position = newPosition;
    chunk.used = true;
    
    int startX = chunk.position.x * chunkWidth;
    int startZ = chunk.position.z * chunkWidth;
    int startY = 0;

    enum Neighbors  {
        LEFT,
        RIGHT,
        FRONT,
        BACK,
        TOP,
        BOTTOM
    };

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

    static std::vector<std::vector<TextureFace>> texs = {
        {TextureFace(0,0),//top
        TextureFace(0,0),//sides
        TextureFace(0,0),//bottom
        }, 
        {TextureFace(0,0),
        TextureFace(0,0),
        TextureFace(0,0),
        }, 
        {TextureFace(1,0),
        TextureFace(1,0),
        TextureFace(1,0),
        }, 
        {TextureFace(3,1),
        TextureFace(3,0),
        TextureFace(2,0),
        }, 
        {TextureFace(2,0),
        TextureFace(2,0),
        TextureFace(2,0),
        },
        {TextureFace(4,0),
        TextureFace(4,0),
        TextureFace(4,0),
        }
    };



    std::vector<float> verts;
    std::vector<float> uvs;

    std::vector<float> tverts;
    std::vector<float> tuvs;

    for(int x = startX; x < startX + chunkWidth; ++x) {
        for(int z = startZ; z < startZ + chunkWidth; ++z) {
            for(int y = startY; y < startY + chunkHeight; ++y) {
                BlockCoord coord(x,y,z);
                int neighborIndex = 0;
                unsigned int block = blockAt(coord);
                if(block != 0) {
                    

                    if(block == 2) {

                        for(BlockCoord &neigh : neighbors) {
                            unsigned int neighblock = blockAt(coord + neigh);
                            if(neighblock == 0) {
                                tverts.insert(tverts.end(), {
                                    faces[neighborIndex][0]+coord.x,faces[neighborIndex][1]+coord.y, faces[neighborIndex][2]+coord.z, faces[neighborIndex][3],
                                    faces[neighborIndex][4]+coord.x,faces[neighborIndex][5]+coord.y, faces[neighborIndex][6]+coord.z, faces[neighborIndex][7],
                                    faces[neighborIndex][8]+coord.x,faces[neighborIndex][9]+coord.y, faces[neighborIndex][10]+coord.z, faces[neighborIndex][11],

                                    faces[neighborIndex][12]+coord.x,faces[neighborIndex][13]+coord.y, faces[neighborIndex][14]+coord.z, faces[neighborIndex][15],
                                    faces[neighborIndex][16]+coord.x,faces[neighborIndex][17]+coord.y, faces[neighborIndex][18]+coord.z, faces[neighborIndex][19],
                                    faces[neighborIndex][20]+coord.x,faces[neighborIndex][21]+coord.y, faces[neighborIndex][22]+coord.z, faces[neighborIndex][23],
                                });
                                if(neighborIndex == TOP) {
                                    tuvs.insert(tuvs.end() , {
                                        texs[block][0].bl.x, texs[block][0].bl.y,
                                        texs[block][0].br.x, texs[block][0].br.y,
                                        texs[block][0].tr.x, texs[block][0].tr.y,

                                        texs[block][0].tr.x, texs[block][0].tr.y,
                                        texs[block][0].tl.x, texs[block][0].tl.y,
                                        texs[block][0].bl.x, texs[block][0].bl.y
                                    });
                                } else if(neighborIndex == BOTTOM) {
                                    tuvs.insert(tuvs.end() , {
                                        texs[block][2].bl.x, texs[block][2].bl.y,
                                        texs[block][2].br.x, texs[block][2].br.y,
                                        texs[block][2].tr.x, texs[block][2].tr.y,

                                        texs[block][2].tr.x, texs[block][2].tr.y,
                                        texs[block][2].tl.x, texs[block][2].tl.y,
                                        texs[block][2].bl.x, texs[block][2].bl.y
                                    });
                                } else {
                                    tuvs.insert(tuvs.end() , {
                                        texs[block][1].bl.x, texs[block][1].bl.y,
                                        texs[block][1].br.x, texs[block][1].br.y,
                                        texs[block][1].tr.x, texs[block][1].tr.y,

                                        texs[block][1].tr.x, texs[block][1].tr.y,
                                        texs[block][1].tl.x, texs[block][1].tl.y,
                                        texs[block][1].bl.x, texs[block][1].bl.y
                                    });
                                }
                            }
                            neighborIndex++;
                        }

                    }else {


                        for(BlockCoord &neigh : neighbors) {
                            unsigned int neighblock = blockAt(coord + neigh);
                            bool solidNeighboringWater = (neighblock == 2 && block != 2);
                            if(neighblock == 0 || solidNeighboringWater) {
                                verts.insert(verts.end(), {
                                    faces[neighborIndex][0]+coord.x,faces[neighborIndex][1]+coord.y, faces[neighborIndex][2]+coord.z, faces[neighborIndex][3],
                                    faces[neighborIndex][4]+coord.x,faces[neighborIndex][5]+coord.y, faces[neighborIndex][6]+coord.z, faces[neighborIndex][7],
                                    faces[neighborIndex][8]+coord.x,faces[neighborIndex][9]+coord.y, faces[neighborIndex][10]+coord.z, faces[neighborIndex][11],

                                    faces[neighborIndex][12]+coord.x,faces[neighborIndex][13]+coord.y, faces[neighborIndex][14]+coord.z, faces[neighborIndex][15],
                                    faces[neighborIndex][16]+coord.x,faces[neighborIndex][17]+coord.y, faces[neighborIndex][18]+coord.z, faces[neighborIndex][19],
                                    faces[neighborIndex][20]+coord.x,faces[neighborIndex][21]+coord.y, faces[neighborIndex][22]+coord.z, faces[neighborIndex][23],
                                });
                                if(neighborIndex == TOP) {
                                    uvs.insert(uvs.end() , {
                                        texs[block][0].bl.x, texs[block][0].bl.y,
                                        texs[block][0].br.x, texs[block][0].br.y,
                                        texs[block][0].tr.x, texs[block][0].tr.y,

                                        texs[block][0].tr.x, texs[block][0].tr.y,
                                        texs[block][0].tl.x, texs[block][0].tl.y,
                                        texs[block][0].bl.x, texs[block][0].bl.y
                                    });
                                } else if(neighborIndex == BOTTOM) {
                                    uvs.insert(uvs.end() , {
                                        texs[block][2].bl.x, texs[block][2].bl.y,
                                        texs[block][2].br.x, texs[block][2].br.y,
                                        texs[block][2].tr.x, texs[block][2].tr.y,

                                        texs[block][2].tr.x, texs[block][2].tr.y,
                                        texs[block][2].tl.x, texs[block][2].tl.y,
                                        texs[block][2].bl.x, texs[block][2].bl.y
                                    });
                                } else {
                                    uvs.insert(uvs.end() , {
                                        texs[block][1].bl.x, texs[block][1].bl.y,
                                        texs[block][1].br.x, texs[block][1].br.y,
                                        texs[block][1].tr.x, texs[block][1].tr.y,

                                        texs[block][1].tr.x, texs[block][1].tr.y,
                                        texs[block][1].tl.x, texs[block][1].tl.y,
                                        texs[block][1].bl.x, texs[block][1].bl.y
                                    });
                                }
                            }
                            neighborIndex++;
                        }

                    }


                }
            }
        }
    }

    try {
        geometryStorePool.at(chunk.geometryStorePoolIndex).verts = verts;
    }
    catch (std::exception e) {
        std::cout << e.what() << "\n";
        std::cout << "index: " << chunk.geometryStorePoolIndex << "\n";
        std::cout << "size: " << geometryStorePool.size() << "\n";
    }
    try {
    geometryStorePool.at(chunk.geometryStorePoolIndex).uvs = uvs;
    }
    catch (std::exception e) {
        std::cout << e.what() << "\n";
        std::cout << "index: " << chunk.geometryStorePoolIndex << "\n";
        std::cout << "size: " << geometryStorePool.size() << "\n";
    }
    try {
    geometryStorePool.at(chunk.geometryStorePoolIndex).tverts = tverts;
    }
    catch (std::exception e) {
        std::cout << e.what() << "\n";
        std::cout << "index: " << chunk.geometryStorePoolIndex << "\n";
        std::cout << "size: " << geometryStorePool.size() << "\n";
    }
    try {
    geometryStorePool.at(chunk.geometryStorePoolIndex).tuvs = tuvs;
    }
    catch (std::exception e) {
        std::cout << e.what() << "\n";
        std::cout << "index: " << chunk.geometryStorePoolIndex << "\n";
        std::cout << "size: " << geometryStorePool.size() << "\n";
    }

    bool found = false;
    for(int i : geometryStoresToRebuild) {
        if(i == chunk.geometryStorePoolIndex) {
            found = true;
        }
    }

    if(!found) {
        geometryStoresToRebuild.push_back(chunk.geometryStorePoolIndex);
    }

    if(takenCareOfChunkSpots.find(chunk.position) == takenCareOfChunkSpots.end()) {
        takenCareOfChunkSpots.insert_or_assign(chunk.position, chunk);
    }

}

















unsigned int VoxelWorld::blockAt(BlockCoord coord) {
    static int waterLevel = 20;
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
        if(noiseFunction(coord.x, coord.y+10, coord.z) > 10) {
            return 5;
        }
        if(coord.y > waterLevel + 2 || noiseFunction(coord.x, coord.y+5, coord.z) > 10) {
            if(noiseFunction(coord.x, coord.y+1, coord.z) < 10) {
                        return 3;
                    }
            return 4;
        } else {
            return 1;
        }
    }
    if(coord.y < waterLevel) {
        return 2;
    }
    return 0;
}

float VoxelWorld::noiseFunction(int x, int y, int z) {
    return 
    std::max(0.0f, (
        20.0f + static_cast<float>(perlin.noise((static_cast<float>(x))/20.35f, (static_cast<float>(y+(seed/100)))/20.35f, (static_cast<float>(z))/20.35f)) * 5.0f
    ) - std::max(((float)y/2.0f) + static_cast<float>(perlin.noise(x/65.0f, z/65.0f)) * 10.0f, 0.0f));
}

bool VoxelWorld::saveExists(const char* path) {
    std::string worldPath(path);
    worldPath += "/world.save";
    return std::filesystem::exists(worldPath);
}

void VoxelWorld::saveWorldToFile(const char *path) {
    std::string worldPath(path);
    worldPath += "/world.save";

    std::filesystem::create_directories(std::filesystem::path(worldPath).parent_path());

    std::ofstream outputFile(worldPath, std::ios::trunc);
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
    std::string worldPath(path);
    worldPath += "/world.save";
    std::ifstream file(worldPath);
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
        file.close();
    } else {
        throw std::exception("Could not open world file.");
    }
}