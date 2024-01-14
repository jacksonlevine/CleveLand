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


void VoxelWorld::chunkUpdateThreadFunction(int* loadRadius) {
    
    std::cout << "Thread started \n";
    glm::vec3 lastCamPosDivided;
    bool first = true;


    while(runChunkThread) {
        glm::vec3 currCamPosDivided = cameraPosition/10.0f;

        


        BlockCoord cameraBlockPos(std::round(cameraPosition.x), std::round(cameraPosition.y), std::round(cameraPosition.z));
        ChunkCoord cameraChunkPos(std::floor(static_cast<float>(cameraBlockPos.x)/chunkWidth), std::floor(static_cast<float>(cameraBlockPos.z)/chunkWidth));

        BlockChunk* chunk = 0;
        while(deferredChunkQueue.pop(chunk)) {
            rebuildChunk(chunk, chunk->position, true);
        }

        if(currCamPosDivided != lastCamPosDivided || first || shouldTryReload) {
            lastCamPosDivided = currCamPosDivided;

            if(first) {
                std::cout << "First \n";
            }

            shouldTryReload = false;


           std::vector<BlockChunk*> sortedChunkPtrs = getPreferredChunkPtrList(*loadRadius, cameraChunkPos);


                int takenChunkIndex = 0;
                for(int x = cameraChunkPos.x - *loadRadius; x < cameraChunkPos.x + *loadRadius; ++x) {
                    for(int z = cameraChunkPos.z - *loadRadius; z < cameraChunkPos.z + *loadRadius; ++z) {

                        ChunkCoord thisChunkCoord(x,z);
                        if(takenCareOfChunkSpots.find(thisChunkCoord) == takenCareOfChunkSpots.end()) {

                            rebuildChunk(sortedChunkPtrs[takenChunkIndex], thisChunkCoord, false);

                            takenChunkIndex++;
                        }
                        if(!runChunkThread || deferredChunksToRebuild.size() > 0) {
                            break;
                        }
                        if(first) {
                            initialLoadProgress += 1;
                        }
                    }

                    if(!runChunkThread || deferredChunksToRebuild.size() > 0) {
                        break;
                    }
                }

            first = false;
        }
    }

    std::cout << "Thread ended \n";
}

std::vector<BlockChunk*> VoxelWorld::getPreferredChunkPtrList(int loadRadius, ChunkCoord& cameraChunkPos) {

         std::vector<BlockChunk*> sortedChunkPtrs;

        // Fill the vector with pointers to the elements of chunks
        for (auto& chunk : chunks) {
            sortedChunkPtrs.push_back(&chunk);
        }

        std::partition(sortedChunkPtrs.begin(), sortedChunkPtrs.end(), [cameraChunkPos, loadRadius](BlockChunk *chunk) {
            
            if(!chunk->used) {
                return true;
            }

            int dist = 
            std::abs(chunk->position.x - cameraChunkPos.x) +
            std::abs(chunk->position.z - cameraChunkPos.z);

            return dist >= loadRadius*2;
        });

        return sortedChunkPtrs;
}

void VoxelWorld::populateChunksAndGeometryStores(entt::registry &registry, int viewDistance) {
    for(int i = 0; i < (viewDistance*2)+10; ++i) {
        for(int j = 0; j < (viewDistance*2)+10; ++j) {

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


void VoxelWorld::rebuildChunk(BlockChunk *chunk, ChunkCoord newPosition, bool immediateInPlace) {
    if(!immediateInPlace) {
        if(takenCareOfChunkSpots.find(chunk->position) != takenCareOfChunkSpots.end()) {
            takenCareOfChunkSpots.erase(chunk->position);
        }
        chunk->position = newPosition;
        generateChunk(newPosition);
    }



    chunk->used = true;
    
    int startX = chunk->position.x * chunkWidth;
    int startZ = chunk->position.z * chunkWidth;
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

    static std::vector<glm::vec3> normals = {
        glm::vec3(-1, 0, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, -1),
        glm::vec3(0, 1, 0),
        glm::vec3(0, -1, 0),
    };

    static std::vector<std::vector<float>> faces = {
           
    {
        -0.5f, -0.5f, 0.5f, 0.0f, 10.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 10.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 10.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 10.0f,
        -0.5f, 0.5f, 0.5f,0.0f, 10.0f,
        -0.5f, -0.5f, 0.5f,0.0f, 10.0f
    },
    {
        0.5f, -0.5f, -0.5f,0.0f, 10.0f,
        0.5f, -0.5f, 0.5f,0.0f, 10.0f,
        0.5f, 0.5f, 0.5f,0.0f, 10.0f,

        0.5f, 0.5f, 0.5f,0.0f, 10.0f,
        0.5f, 0.5f, -0.5f,0.0f, 10.0f,
        0.5f, -0.5f, -0.5f,0.0f, 10.0f
    },
    {
        0.5f, -0.5f, 0.5f, 0.0f,14.0f,
        -0.5f, -0.5f, 0.5f,0.0f, 14.0f,
        -0.5f, 0.5f, 0.5f,0.0f, 14.0f,

        -0.5f, 0.5f, 0.5f,0.0f, 14.0f,
        0.5f, 0.5f, 0.5f,0.0f, 14.0f,
        0.5f, -0.5f, 0.5f, 0.0f,14.0f
    },
    {
        -0.5f, -0.5f, -0.5f,0.0f,  14.0f,
        0.5f, -0.5f, -0.5f,0.0f, 14.0f,
        0.5f, 0.5f, -0.5f,0.0f, 14.0f,

        0.5f, 0.5f, -0.5f,0.0f, 14.0f,
        -0.5f, 0.5f, -0.5f,0.0f, 14.0f,
        -0.5f, -0.5f, -0.5f,0.0f, 14.0f
    },
     {
        -0.5f, 0.5f, -0.5f,0.0f, 16.0f,
        0.5f, 0.5f, -0.5f,0.0f, 16.0f,
        0.5f, 0.5f, 0.5f,0.0f, 16.0f,

        0.5f, 0.5f, 0.5f, 0.0f,16.0f,
        -0.5f, 0.5f, 0.5f,0.0f, 16.0f,
        -0.5f, 0.5f, -0.5f,0.0f, 16.0f,
    },
    {
        0.5f, -0.5f, -0.5f, 0.0f,7.0f,
        -0.5f, -0.5f, -0.5f,0.0f, 7.0f,
        -0.5f, -0.5f, 0.5f,0.0f, 7.0f,

        -0.5f, -0.5f, 0.5f,0.0f, 7.0f,
        0.5f, -0.5f, 0.5f,0.0f, 7.0f,
        0.5f, -0.5f, -0.5f,0.0f,  7.0f
    }
    };


    static std::vector<float> doorBottomUVs = DoorInfo::getDoorUVs(TextureFace(11,0));
    static std::vector<float> doorTopUVs = DoorInfo::getDoorUVs(TextureFace(11,1));


    std::vector<float> verts;
    std::vector<float> uvs;

    std::vector<float> tverts;
    std::vector<float> tuvs;

    for(int x = startX; x < startX + chunkWidth; ++x) {
        for(int z = startZ; z < startZ + chunkWidth; ++z) {
            for(int y = startY; y < startY + chunkHeight; ++y) {
                BlockCoord coord(x,y,z);
                int neighborIndex = 0;
                uint32_t combined = blockAt(coord);
                uint32_t block = combined & BlockInfo::BLOCK_ID_BITS;
                uint32_t flags = combined & BlockInfo::BLOCK_FLAG_BITS;
                if(block != 0) {
                    
                    if(block == 11) {
                        int direction = BlockInfo::getDirectionBits(flags);
                        int open = DoorInfo::getDoorOpenBit(flags);
                        int opposite = DoorInfo::getOppositeDoorBits(flags);

                        int modelIndex;
                        if(opposite == 1) {
                            modelIndex = (direction - open);
                            if(modelIndex < 0) {
                                modelIndex = 3;
                            }
                        } else {
                            modelIndex = (direction + open) % 4;
                        }

                        int doorTop = DoorInfo::getDoorTopBit(flags);
                        
                        int index = 0;
                        for(float vert : DoorInfo::doorModels[modelIndex]) {
                            float thisvert = 0.0f;
                            if(index == 0){
                                thisvert = vert + coord.x;
                            } else
                            if(index == 1){
                                thisvert = vert + coord.y;
                            } else
                            if(index == 2){
                                thisvert = vert + coord.z;
                            } else {
                                thisvert = vert;
                            }
                            tverts.push_back(thisvert);
                            index = (index + 1) % 5;
                        }

                        if(doorTop) {
                            tuvs.insert(tuvs.end(),doorTopUVs.begin(), doorTopUVs.end());
                        } else {
                            tuvs.insert(tuvs.end(),doorBottomUVs.begin(), doorBottomUVs.end());
                        }
                    } else
                    if(std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), block) != BlockInfo::transparents.end()) {

                        for(BlockCoord &neigh : neighbors) {
                            uint32_t neighblockcombined = blockAt(coord + neigh);
                            uint32_t neighblock = neighblockcombined & BlockInfo::BLOCK_ID_BITS;
                            if(neighblock == 0) {
                                tverts.insert(tverts.end(), {
                                    faces[neighborIndex][0] + coord.x, faces[neighborIndex][1] + coord.y, faces[neighborIndex][2] + coord.z, faces[neighborIndex][3],
                                    faces[neighborIndex][4], 
                                    faces[neighborIndex][5] + coord.x, faces[neighborIndex][6] + coord.y, faces[neighborIndex][7] + coord.z, faces[neighborIndex][8],
                                    faces[neighborIndex][9], 
                                    faces[neighborIndex][10] + coord.x, faces[neighborIndex][11] + coord.y, faces[neighborIndex][12] + coord.z, faces[neighborIndex][13],
                                    faces[neighborIndex][14], 
                                    faces[neighborIndex][15] + coord.x, faces[neighborIndex][16] + coord.y, faces[neighborIndex][17] + coord.z, faces[neighborIndex][18],
                                    faces[neighborIndex][19], 
                                    faces[neighborIndex][20] + coord.x, faces[neighborIndex][21] + coord.y, faces[neighborIndex][22] + coord.z, faces[neighborIndex][23],
                                    faces[neighborIndex][24], 
                                    faces[neighborIndex][25] + coord.x, faces[neighborIndex][26] + coord.y, faces[neighborIndex][27] + coord.z, faces[neighborIndex][28],
                                    faces[neighborIndex][29]
                                });
                                if(neighborIndex == TOP) {
                                    tuvs.insert(tuvs.end() , {
                                        BlockInfo::texs[block][0].bl.x, BlockInfo::texs[block][0].bl.y,
                                        BlockInfo::texs[block][0].br.x, BlockInfo::texs[block][0].br.y,
                                        BlockInfo::texs[block][0].tr.x, BlockInfo::texs[block][0].tr.y,

                                        BlockInfo::texs[block][0].tr.x, BlockInfo::texs[block][0].tr.y,
                                        BlockInfo::texs[block][0].tl.x, BlockInfo::texs[block][0].tl.y,
                                        BlockInfo::texs[block][0].bl.x, BlockInfo::texs[block][0].bl.y
                                    });
                                } else if(neighborIndex == BOTTOM) {
                                    tuvs.insert(tuvs.end() , {
                                        BlockInfo::texs[block][2].bl.x, BlockInfo::texs[block][2].bl.y,
                                        BlockInfo::texs[block][2].br.x, BlockInfo::texs[block][2].br.y,
                                        BlockInfo::texs[block][2].tr.x, BlockInfo::texs[block][2].tr.y,

                                        BlockInfo::texs[block][2].tr.x, BlockInfo::texs[block][2].tr.y,
                                        BlockInfo::texs[block][2].tl.x, BlockInfo::texs[block][2].tl.y,
                                        BlockInfo::texs[block][2].bl.x, BlockInfo::texs[block][2].bl.y
                                    });
                                } else {
                                    tuvs.insert(tuvs.end() , {
                                        BlockInfo::texs[block][1].bl.x, BlockInfo::texs[block][1].bl.y,
                                        BlockInfo::texs[block][1].br.x, BlockInfo::texs[block][1].br.y,
                                        BlockInfo::texs[block][1].tr.x, BlockInfo::texs[block][1].tr.y,

                                        BlockInfo::texs[block][1].tr.x, BlockInfo::texs[block][1].tr.y,
                                        BlockInfo::texs[block][1].tl.x, BlockInfo::texs[block][1].tl.y,
                                        BlockInfo::texs[block][1].bl.x, BlockInfo::texs[block][1].bl.y
                                    });
                                }
                            }
                            neighborIndex++;
                        }

                    }else {


                        for(BlockCoord &neigh : neighbors) {
                            uint32_t neighblockcombined = blockAt(coord + neigh);
                            uint32_t neighblock = neighblockcombined & BlockInfo::BLOCK_ID_BITS;
                            bool solidNeighboringTransparent = (std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), neighblock) != BlockInfo::transparents.end() && std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), block) == BlockInfo::transparents.end());
                            if(neighblock == 0 || solidNeighboringTransparent) {
                                BlockCoord lightCubeHere = coord + neigh;
                                verts.insert(verts.end(), {
                                    faces[neighborIndex][0] + coord.x, faces[neighborIndex][1] + coord.y, faces[neighborIndex][2] + coord.z, faces[neighborIndex][3],
                                    faces[neighborIndex][4], 
                                    faces[neighborIndex][5] + coord.x, faces[neighborIndex][6] + coord.y, faces[neighborIndex][7] + coord.z, faces[neighborIndex][8],
                                    faces[neighborIndex][9], 
                                    faces[neighborIndex][10] + coord.x, faces[neighborIndex][11] + coord.y, faces[neighborIndex][12] + coord.z, faces[neighborIndex][13],
                                    faces[neighborIndex][14], 
                                    faces[neighborIndex][15] + coord.x, faces[neighborIndex][16] + coord.y, faces[neighborIndex][17] + coord.z, faces[neighborIndex][18],
                                    faces[neighborIndex][19], 
                                    faces[neighborIndex][20] + coord.x, faces[neighborIndex][21] + coord.y, faces[neighborIndex][22] + coord.z, faces[neighborIndex][23],
                                    faces[neighborIndex][24], 
                                    faces[neighborIndex][25] + coord.x, faces[neighborIndex][26] + coord.y, faces[neighborIndex][27] + coord.z, faces[neighborIndex][28],
                                    faces[neighborIndex][29]
                                });
                                if(neighborIndex == TOP) {
                                    uvs.insert(uvs.end() , {
                                        BlockInfo::texs[block][0].bl.x, BlockInfo::texs[block][0].bl.y,
                                        BlockInfo::texs[block][0].br.x, BlockInfo::texs[block][0].br.y,
                                        BlockInfo::texs[block][0].tr.x, BlockInfo::texs[block][0].tr.y,

                                        BlockInfo::texs[block][0].tr.x, BlockInfo::texs[block][0].tr.y,
                                        BlockInfo::texs[block][0].tl.x, BlockInfo::texs[block][0].tl.y,
                                        BlockInfo::texs[block][0].bl.x, BlockInfo::texs[block][0].bl.y
                                    });
                                } else if(neighborIndex == BOTTOM) {
                                    uvs.insert(uvs.end() , {
                                        BlockInfo::texs[block][2].bl.x, BlockInfo::texs[block][2].bl.y,
                                        BlockInfo::texs[block][2].br.x, BlockInfo::texs[block][2].br.y,
                                        BlockInfo::texs[block][2].tr.x, BlockInfo::texs[block][2].tr.y,

                                        BlockInfo::texs[block][2].tr.x, BlockInfo::texs[block][2].tr.y,
                                        BlockInfo::texs[block][2].tl.x, BlockInfo::texs[block][2].tl.y,
                                        BlockInfo::texs[block][2].bl.x, BlockInfo::texs[block][2].bl.y
                                    });
                                } else {
                                    uvs.insert(uvs.end() , {
                                        BlockInfo::texs[block][1].bl.x, BlockInfo::texs[block][1].bl.y,
                                        BlockInfo::texs[block][1].br.x, BlockInfo::texs[block][1].br.y,
                                        BlockInfo::texs[block][1].tr.x, BlockInfo::texs[block][1].tr.y,

                                        BlockInfo::texs[block][1].tr.x, BlockInfo::texs[block][1].tr.y,
                                        BlockInfo::texs[block][1].tl.x, BlockInfo::texs[block][1].tl.y,
                                        BlockInfo::texs[block][1].bl.x, BlockInfo::texs[block][1].bl.y
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

    geometryStorePool.at(chunk->geometryStorePoolIndex).myLock.lock();

        try {
            geometryStorePool.at(chunk->geometryStorePoolIndex).verts = verts;
        }
        catch (std::exception e) {
            std::cout << e.what() << "\n";
            std::cout << "index: " << chunk->geometryStorePoolIndex << "\n";
            std::cout << "size: " << geometryStorePool.size() << "\n";
        }
        try {
        geometryStorePool.at(chunk->geometryStorePoolIndex).uvs = uvs;
        }
        catch (std::exception e) {
            std::cout << e.what() << "\n";
            std::cout << "index: " << chunk->geometryStorePoolIndex << "\n";
            std::cout << "size: " << geometryStorePool.size() << "\n";
        }
        try {
        geometryStorePool.at(chunk->geometryStorePoolIndex).tverts = tverts;
        }
        catch (std::exception e) {
            std::cout << e.what() << "\n";
            std::cout << "index: " << chunk->geometryStorePoolIndex << "\n";
            std::cout << "size: " << geometryStorePool.size() << "\n";
        }
        try {
        geometryStorePool.at(chunk->geometryStorePoolIndex).tuvs = tuvs;
        }
        catch (std::exception e) {
            std::cout << e.what() << "\n";
            std::cout << "index: " << chunk->geometryStorePoolIndex << "\n";
            std::cout << "size: " << geometryStorePool.size() << "\n";
        }

        geometryStorePool.at(chunk->geometryStorePoolIndex).myLock.unlock();

    if(!immediateInPlace) {

        // bool found = false;
        // for(int i : geometryStoresToRebuild) {
        //     if(i == chunk->geometryStorePoolIndex) {
        //         found = true;
        //     }
        // }

        // if(!found) {
        //     geometryStoresToRebuild.push_back(chunk->geometryStorePoolIndex);
        // }

        while(!geometryStoreQueue.push(chunk->geometryStorePoolIndex)) {

        }
        if(takenCareOfChunkSpots.find(chunk->position) == takenCareOfChunkSpots.end()) {
            takenCareOfChunkSpots.insert_or_assign(chunk->position, chunk);
        }
    } else {

        while(!highPriorityGeometryStoreQueue.push(chunk->geometryStorePoolIndex)) {

        }


    }


}












void VoxelWorld::generateChunk(ChunkCoord chunkcoord) {

    srand(seed + chunkcoord.x + chunkcoord.y);

    static BlockCoord leafSpots[] = {
        BlockCoord(-1,0,0),
        BlockCoord(1,0,0),
        BlockCoord(0,0,1),
        BlockCoord(0,0,-1),

        BlockCoord(-1,0,1),
        BlockCoord(1,0,1),
        BlockCoord(1,0,-1),
        BlockCoord(-1,0,-1),




        BlockCoord(-1,-1,0),
        BlockCoord(1,-1,0),
        BlockCoord(0,-1,1),
        BlockCoord(0,-1,-1),

        BlockCoord(-1,-1,1),
        BlockCoord(1,-1,1),
        BlockCoord(1,-1,-1),
        BlockCoord(-1,-1,-1),

        BlockCoord(-1,-2,0),
        BlockCoord(1,-2,0),
        BlockCoord(0,-2,1),
        BlockCoord(0,-2,-1),

        BlockCoord(-1,-2,1),
        BlockCoord(1,-2,1),
        BlockCoord(1,-2,-1),
        BlockCoord(-1,-2,-1),


        BlockCoord(-2,-2,-2),
        BlockCoord(-2,-2,-1),
        BlockCoord(-2,-2,-0),
        BlockCoord(-2,-2,1),
        BlockCoord(-2,-2,2),

        BlockCoord(-1,-2,-2),
        BlockCoord(-1,-2,2),

        BlockCoord(0,-2,-2),
        BlockCoord(0,-2,2),

        BlockCoord(1,-2,-2),
        BlockCoord(1,-2,2),

        BlockCoord(2,-2,-2),
        BlockCoord(2,-2,-1),
        BlockCoord(2,-2,-0),
        BlockCoord(2,-2,1),
        BlockCoord(2,-2,2),

        BlockCoord(0,1,0),
        BlockCoord(-1,1,0),
        BlockCoord(1,1,0),
        BlockCoord(0,1,1),
        BlockCoord(0,1,-1),
    };

    std::set<BlockChunk*> implicatedChunks;

    for(int x = 0; x < chunkWidth; ++x) {
        for(int z = 0; z < chunkWidth; ++z) {
            for(int y = 0; y < chunkHeight; ++y) {
                BlockCoord coord(chunkcoord.x * chunkWidth + x, y, chunkcoord.z * chunkWidth + z);

                //Trees
                if(blockAt(coord) == 3) {
                    int ran = randsmall();
                    if(ran > 126) {
                        BlockCoord here = coord;
                        int height = 5 + static_cast<int>(4.0f*rando());
                        for(int i = 0; i < height; ++i) {
                            
                            here += BlockCoord(0,1,0);
                            bool doingIt = true;
                            auto chunkIt = userDataMap.find(chunkcoord);
                            if(chunkIt != userDataMap.end()) {
                                if(chunkIt->second.find(here) != chunkIt->second.end()) {
                                    doingIt = false;
                                }
                            }
                            if(doingIt) {
                                nonUserDataMap.insert_or_assign(here, 6);
                            }
                        }
                        for(BlockCoord& coor : leafSpots) {
                            BlockCoord coorHere = here + coor;
                            bool doingIt = true;
                            auto chunkIt = userDataMap.find(chunkcoord);
                            if(chunkIt != userDataMap.end()) {
                                if(chunkIt->second.find(coorHere) != chunkIt->second.end()) {
                                    doingIt = false;
                                }
                            }
                            if(doingIt) {
                                ChunkCoord chunkHere(
                                    std::floor(static_cast<float>(coorHere.x)/chunkWidth),
                                    std::floor(static_cast<float>(coorHere.z)/chunkWidth)
                                );
                                if(nonUserDataMap.find(coorHere) == nonUserDataMap.end()) {
                                    nonUserDataMap.insert_or_assign(coorHere, 7);
                                    auto chunkIt = takenCareOfChunkSpots.find(chunkHere);
                                    if(chunkIt != takenCareOfChunkSpots.end()) {
                                        implicatedChunks.insert(chunkIt->second);
                                    }
                                }
                            }
                        }
                    }
                }






            }
        }
    }

    for(BlockChunk *pointer : implicatedChunks) {
        while(!deferredChunkQueue.push(pointer)) {

        }
    }

        
}




uint32_t VoxelWorld::blockAt(BlockCoord coord) {
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
    auto blockit = nonUserDataMap.find(coord);
    if(blockit != nonUserDataMap.end()) {
        return blockit->second;
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
                    std::unordered_map<BlockCoord, uint32_t, IntTupHash> thisBlockMap;
                    userDataMap.insert_or_assign(currentChunkCoord, thisBlockMap);
                } else {
                    std::string word;
                    int localIndex = 0;
                    BlockCoord thisBlockCoord;
                    uint32_t thisID;
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
                            thisID = static_cast<uint32_t>(std::stoi(word));
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