#include "voxelworld.h"


void VoxelWorld::setBlock(BlockCoord coord, uint32_t block) {
    ChunkCoord cc(
        std::floor(static_cast<float>(coord.x) / chunkWidth),
        std::floor(static_cast<float>(coord.z) / chunkWidth)
    );
    udmMutex.lock();  
            
    auto chunkIt = userDataMap.find(cc);

    if(chunkIt == userDataMap.end()) {
        userDataMap.insert_or_assign(cc, std::unordered_map<BlockCoord, uint32_t, IntTupHash>());
    }
    userDataMap.at(cc).insert_or_assign(coord, block);
    
    udmMutex.unlock();  

    (*multiplayerBlockSetFunc)(coord.x, coord.y, coord.z, block);
}


void VoxelWorld::setBlockAndQueueRerender(BlockCoord coord, uint32_t block) {
    setBlock(coord, block);
    ChunkCoord cc(
        coord.x / chunkWidth,
        coord.z / chunkWidth
    );
    std::set<BlockChunk *> implicated;

    for(BlockCoord& neigh : BlockInfo::neighbors) {
        auto segIt = lightMap.find(coord + neigh);
        if(segIt != lightMap.end()) {
            
            for(LightRay& ray : segIt->second.rays) {
                ChunkCoord chunkOfOrigin(
                    std::floor(static_cast<float>(ray.origin.x)/chunkWidth),
                    std::floor(static_cast<float>(ray.origin.z)/chunkWidth)
                );
                auto chunkIt = takenCareOfChunkSpots.find(chunkOfOrigin);
                if(chunkIt != takenCareOfChunkSpots.end()){
                    implicated.insert(chunkIt->second);
                }
                    
            }
            
        }
    }
    for(BlockChunk * pointer : implicated) {
        while(!lightUpdateQueue.push(pointer)) {

        }
        //std::cout << "Doing this\n";
    }


    auto chunkIt = takenCareOfChunkSpots.find(cc);
    if(chunkIt != takenCareOfChunkSpots.end()) {
        BlockChunk *chunk = chunkIt->second;
        while(!deferredChunkQueue.push(chunk)) {

        }
    }
}


void VoxelWorld::runStep(float deltaTime) {
    static float timer = 0.0f;

    if(timer > 5.0f) {
        shouldTryReload = true;

        timer = 0.0f;
    } else {
        timer += deltaTime;
    }
}

float VoxelWorld::noiseFunction(int x, int y, int z) {
        return 
        std::max(0.0f, (
            20.0f + static_cast<float>(perlin.noise((static_cast<float>(x))/20.35f, (static_cast<float>(y+(seed/100)))/20.35f, (static_cast<float>(z))/20.35f)) * 5.0f
        ) - std::max(((float)y/2.0f) + static_cast<float>(perlin.noise(x/65.0f, z/65.0f)) * 10.0f, 0.0f));
    }

float VoxelWorld::noiseFunction2(int x, int y, int z) {

        float noise1;
        float noise2;

        y -= 20;

        noise1 =
        std::max(0.0f, (
            20.0f + static_cast<float>(perlin.noise((static_cast<float>(x+worldOffset.x))/25.35f, (static_cast<float>(y))/20.35f, (static_cast<float>(z+worldOffset.z))/25.35f)) * 5.0f
        ) - std::max(((float)y/2.0f) + static_cast<float>(perlin.noise(x/65.0f, z/65.0f)) * 10.0f, 0.0f));

        y += 60;

        noise2 =
        std::max(0.0f, (
             50.0f + static_cast<float>(perlin.noise((static_cast<float>(x+worldOffset.x))/55.35f, (static_cast<float>(y+worldOffset.y))/25.35f, (static_cast<float>(z+worldOffset.z))/55.35f)) * 10.0f
            + static_cast<float>(perlin.noise((static_cast<float>(x+10000+worldOffset.x))/25.35f, (static_cast<float>(y+worldOffset.y))/65.35f, (static_cast<float>(z+10000+worldOffset.z))/25.35f)) * 20.0f

        ) - std::max(((float)y/3.0f) /*+ static_cast<float>(perlin.noise(x/15.0f, z/15.0f)) * 2.0f*/, 0.0f));

         float p = perlin.noise((x+worldOffset.x)/500.0f, (z+worldOffset.z)/500.0f) * 10.0f;
            
        p = std::max(p, 0.0f);
        p = std::min(p, 1.0f);

        return glm::mix(noise1, noise2, p*0.5f);



    }


void VoxelWorld::getOffsetFromSeed() {
    srand(seed);
    worldOffset = glm::ivec3(rand()*100, rand()*100, rand()*100);
}

#define DEV

void VoxelWorld::chunkUpdateThreadFunction(int loadRadius) {
    stillRunningThread = true;
    #ifdef DEV
        std::cout << "Thread started \n";
    #endif
    glm::vec3 lastCamPosDivided;
    bool first = true;


    while(runChunkThread.load()) {
        glm::vec3 currCamPosDivided = cameraPosition/10.0f;

        

        BlockCoord cameraBlockPos(std::round(cameraPosition.x), std::round(cameraPosition.y), std::round(cameraPosition.z));
        ChunkCoord cameraChunkPos(std::floor(static_cast<float>(cameraBlockPos.x) / chunkWidth), std::floor(static_cast<float>(cameraBlockPos.z) / chunkWidth));


        

        BlockChunk* chunk2 = 0;
        while(lightUpdateQueue.pop(chunk2)) {
            if(!runChunkThread.load()) {
                break;
            }
            rebuildChunk(chunk2, chunk2->position, true, true);

        }

        BlockChunk* chunk = 0;
        while(deferredChunkQueue.pop(chunk)) {
            if(!runChunkThread.load()) {
                break;
            }
            rebuildChunk(chunk, chunk->position, true, false);
        }

        if(currCamPosDivided != lastCamPosDivided || first || shouldTryReload) {
            lastCamPosDivided = currCamPosDivided;

            #ifdef DEV
            if(first) {
                std::cout << "First \n";
            }
            #endif

            shouldTryReload = false;


           std::vector<BlockChunk*> sortedChunkPtrs = getPreferredChunkPtrList(loadRadius, cameraChunkPos);


                int takenChunkIndex = 0;
                for(int x = cameraChunkPos.x - loadRadius; x < cameraChunkPos.x + loadRadius; ++x) {
                    for(int z = cameraChunkPos.z - loadRadius; z < cameraChunkPos.z + loadRadius; ++z) {

                        ChunkCoord thisChunkCoord(x,z);
                        if(takenCareOfChunkSpots.find(thisChunkCoord) == takenCareOfChunkSpots.end()) {
                            
                            if(hasHadInitialLightPass.find(thisChunkCoord) == hasHadInitialLightPass.end()) {
                                rebuildChunk(sortedChunkPtrs[takenChunkIndex], thisChunkCoord, false, true);
                            } else {
                                rebuildChunk(sortedChunkPtrs[takenChunkIndex], thisChunkCoord, false, false);
                            }
                            

                            takenChunkIndex++;
                        }
                        
                        if(!runChunkThread.load()) {
                            break;
                        }
                        if(first) {
                            initialLoadProgress += 1;
                        } else {
                            if(deferredChunkQueue.pop(chunk)) {
                                rebuildChunk(chunk, chunk->position, true, false);
                                break;
                            }
                        }
                    }
                    if(!runChunkThread.load() ) {
                        break;
                    }
                    if(!first) {
                        if(deferredChunkQueue.pop(chunk)) {
                                rebuildChunk(chunk, chunk->position, true, false);
                                break;
                        }
                    }
                }

            first = false;
        }
    }

    #ifdef DEV
    std::cout << "Thread ended \n";
    #endif
    stillRunningThread = false;
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


void VoxelWorld::rebuildChunk(BlockChunk *chunk, ChunkCoord newPosition, bool immediateInPlace, bool light) {



    float time = glfwGetTime();

    std::unordered_map<BlockCoord, uint32_t, IntTupHash> memo;

    if(!immediateInPlace) {
        if(takenCareOfChunkSpots.find(chunk->position) != takenCareOfChunkSpots.end()) {
            takenCareOfChunkSpots.erase(chunk->position);
        }
        chunk->position = newPosition;
        generateChunk(newPosition, memo);
    }
    if(light) {
        lightPassOnChunk(newPosition, memo);
    }


    chunk->used = true;

    static std::vector<std::vector<float>> faces = {
           
    {
        -0.5f, -0.5f, 1.5f, 0.0f, 10.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 10.0f,
        -0.5f, 1.5f, -0.5f, 0.0f, 10.0f,

        // -0.5f, 1.5f, -0.5f, 0.0f, 10.0f,
        // -0.5f, 0.5f, 0.5f,0.0f, 10.0f,
        // -0.5f, -0.5f, 0.5f,0.0f, 10.0f
    },
    {
        0.5f, -0.5f, -1.5f,0.0f, 10.0f,
        0.5f, -0.5f, 0.5f,0.0f, 10.0f,
        0.5f, 1.5f, 0.5f,0.0f, 10.0f,

        // 0.5f, 0.5f, 0.5f,0.0f, 10.0f,
        // 0.5f, 0.5f, -0.5f,0.0f, 10.0f,
        // 0.5f, -0.5f, -0.5f,0.0f, 10.0f
    },
    {
        1.5f, -0.5f, 0.5f, 0.0f,14.0f,
        -0.5f, -0.5f, 0.5f,0.0f, 14.0f,
        -0.5f, 1.5f, 0.5f,0.0f, 14.0f,

        // -0.5f, 0.5f, 0.5f,0.0f, 14.0f,
        // 0.5f, 0.5f, 0.5f,0.0f, 14.0f,
        // 0.5f, -0.5f, 0.5f, 0.0f,14.0f
    },
    {
        -1.5f, -0.5f, -0.5f,0.0f,  14.0f,
        0.5f, -0.5f, -0.5f,0.0f, 14.0f,
        0.5f, 1.5f, -0.5f,0.0f, 14.0f,

        // 0.5f, 0.5f, -0.5f,0.0f, 14.0f,
        // -0.5f, 0.5f, -0.5f,0.0f, 14.0f,
        // -0.5f, -0.5f, -0.5f,0.0f, 14.0f
    },
     {
        -1.5f, 0.5f, -0.5f,0.0f, 16.0f,
        0.5f, 0.5f, -0.5f,0.0f, 16.0f,
        0.5f, 0.5f, 1.5f,0.0f, 16.0f,

        // 0.5f, 0.5f, 0.5f, 0.0f,16.0f,
        // -0.5f, 0.5f, 0.5f,0.0f, 16.0f,
        // -0.5f, 0.5f, -0.5f,0.0f, 16.0f,
    },
    {
        1.5f, -0.5f, -0.5f, 0.0f,7.0f,
        -0.5f, -0.5f, -0.5f,0.0f, 7.0f,
        -0.5f, -0.5f, 1.5f,0.0f, 7.0f,

        // -0.5f, -0.5f, 0.5f,0.0f, 7.0f,
        // 0.5f, -0.5f, 0.5f,0.0f, 7.0f,
        // 0.5f, -0.5f, -0.5f,0.0f,  7.0f
    }
    };


    static std::vector<float> doorBottomUVs = DoorInfo::getDoorUVs(TextureFace(11,0));
    static std::vector<float> doorTopUVs = DoorInfo::getDoorUVs(TextureFace(11,1));





    std::vector<float> verts;
    std::vector<float> uvs;

    std::vector<float> tverts;
    std::vector<float> tuvs;

        
    int startX = chunk->position.x * chunkWidth;
    int startZ = chunk->position.z * chunkWidth;
    int startY = 0;









    for(int x = startX; x < startX + chunkWidth; ++x) {
        for(int z = startZ; z < startZ + chunkWidth; ++z) {
            for(int y = startY; y < startY + chunkHeight; ++y) {
                BlockCoord coord(x,y,z);
                int neighborIndex = 0;
                uint32_t combined = blockAtMemo(coord, memo);
                uint32_t block = combined & BlockInfo::BLOCK_ID_BITS;
                uint32_t flags = combined & BlockInfo::BLOCK_FLAG_BITS;
                if(block != 0) {


                    bool semiTrans = std::find(BlockInfo::semiTransparents.begin(), BlockInfo::semiTransparents.end(), block) != BlockInfo::semiTransparents.end();
                    
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

                        float blockLightVal = 0.0f;
                        auto segIt = lightMap.find(coord);
                        if(segIt != lightMap.end()) {
                            for(LightRay& ray : segIt->second.rays) {
                                blockLightVal = std::min(blockLightVal + ray.value, 16.0f);
                            }
                        }
                        
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
                            } else 
                            if(index == 3) {
                                thisvert = vert + blockLightVal;
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
                    if(block == 14) {

                        static std::vector<float> ladderUVs = LadderInfo::getLadderUVs();
                        int direction = BlockInfo::getDirectionBits(flags);

                        int modelIndex = direction;

                        float blockLightVal = 0.0f;
                        auto segIt = lightMap.find(coord);
                        if(segIt != lightMap.end()) {
                            for(LightRay& ray : segIt->second.rays) {
                                blockLightVal = std::min(blockLightVal + ray.value, 16.0f);
                            }
                        }
                        
                        int index = 0;
                        for(float vert : LadderInfo::ladderModels[modelIndex]) {
                            float thisvert = 0.0f;
                            if(index == 0){
                                thisvert = vert + coord.x;
                            } else
                            if(index == 1){
                                thisvert = vert + coord.y;
                            } else
                            if(index == 2){
                                thisvert = vert + coord.z;
                            } else 
                            if(index == 3) {
                                thisvert = vert + blockLightVal;
                            } else {
                                thisvert = vert;
                            }
                            tverts.push_back(thisvert);
                            index = (index + 1) % 5;
                        }

                        tuvs.insert(tuvs.end(), ladderUVs.begin(), ladderUVs.end());
                    } else
                    if(block == 13) {

                        static std::vector<float> chestUVs = ChestInfo::getChestUVs();
                        int direction = BlockInfo::getDirectionBits(flags);

                        int modelIndex = direction;

                        float blockLightVal = 0.0f;
                        auto segIt = lightMap.find(coord);
                        if(segIt != lightMap.end()) {
                            for(LightRay& ray : segIt->second.rays) {
                                blockLightVal = std::min(blockLightVal + ray.value, 16.0f);
                            }
                        }
                        
                        int index = 0;
                        for(float vert : ChestInfo::chestModels[modelIndex]) {
                            float thisvert = 0.0f;
                            if(index == 0){
                                thisvert = vert + coord.x;
                            } else
                            if(index == 1){
                                thisvert = vert + coord.y;
                            } else
                            if(index == 2){
                                thisvert = vert + coord.z;
                            } else 
                            if(index == 3) {
                                thisvert = vert + blockLightVal;
                            } else {
                                thisvert = vert;
                            }
                            verts.push_back(thisvert);
                            index = (index + 1) % 5;
                        }

                        uvs.insert(uvs.end(), chestUVs.begin(), chestUVs.end());
                    } else
                    if(std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), block) != BlockInfo::transparents.end() ||
                    semiTrans) {

                        for(BlockCoord &neigh : BlockInfo::neighbors) {
                            uint32_t neighblockcombined = blockAtMemo(coord + neigh, memo);
                            uint32_t neighblock = neighblockcombined & BlockInfo::BLOCK_ID_BITS;
                            bool neighSemiTrans = std::find(BlockInfo::semiTransparents.begin(), BlockInfo::semiTransparents.end(), neighblock) != BlockInfo::semiTransparents.end();
                            bool waterBorderingTransparent = (block == 2 && neighblock != 2);
                            if(neighblock == 0 || neighSemiTrans || waterBorderingTransparent) {

                                BlockCoord lightCubeHere = coord + neigh;
                                //std::cout << lightCubeHere.x << " " << lightCubeHere.y << " " << lightCubeHere.z << "\n";
                                bool skyBlocked = false;
                                int yTest = lightCubeHere.y;
                                if(neighborIndex != BOTTOM) {
                                    while(yTest < chunkHeight) {
                                        yTest++;
                                        BlockCoord test(lightCubeHere.x, yTest, lightCubeHere.z);
                                        uint32_t blockHere = blockAtMemo(test, memo);
                                        uint32_t blockIDHere = blockHere & BlockInfo::BLOCK_ID_BITS;
                                        if(blockIDHere != 0 && std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), blockIDHere) == BlockInfo::transparents.end()) {
                                            skyBlocked = true;
                                            break;
                                        }
                                    }
                                }

                                float blockLightVal = 0.0f;
                                auto segIt = lightMap.find(lightCubeHere);
                                if(segIt != lightMap.end()) {
                                    for(LightRay& ray : segIt->second.rays) {
                                        blockLightVal = std::min(blockLightVal + ray.value, 16.0f);
                                    }
                                }

                                float blockShift = 0.0f;
                                float blockBottomShift = 0.0f;
                                float bottomTextureShift = 0.0f;
                                if(block == 2 && blockAtMemo(coord + BlockCoord(0,1,0), memo) != 2) {
                                    blockShift = -0.125f;
                                    blockBottomShift = 0.125f;
                                    bottomTextureShift = 2/288.0f;
                                }

                                

                                std::vector<float> &thisFace = faces[neighborIndex];
                                
                                std::vector<TextureFace> &thisTex = BlockInfo::texs[block];

                                if(neighborIndex == TOP) {

                                    tverts.insert(tverts.end(), {
                                    thisFace[0] + coord.x, thisFace[1] + coord.y + blockShift, thisFace[2] + coord.z, thisFace[3]+blockLightVal,
                                    thisFace[4]+ (skyBlocked ? -4.0f : 0.0f), 
                                    thisFace[5] + coord.x, thisFace[6] + coord.y + blockShift, thisFace[7] + coord.z, thisFace[8]+blockLightVal,
                                    thisFace[9]+ (skyBlocked ? -4.0f : 0.0f), 
                                    thisFace[10] + coord.x, thisFace[11] + coord.y + blockShift, thisFace[12] + coord.z, thisFace[13]+blockLightVal,
                                    thisFace[14]+ (skyBlocked ? -4.0f : 0.0f), 




                                    // thisFace[15] + coord.x, thisFace[16] + coord.y + blockShift, thisFace[17] + coord.z, thisFace[18]+blockLightVal,
                                    // thisFace[19]+ (skyBlocked ? -4.0f : 0.0f), 
                                    // thisFace[20] + coord.x, thisFace[21] + coord.y + blockShift, thisFace[22] + coord.z, thisFace[23]+blockLightVal,
                                    // thisFace[24]+ (skyBlocked ? -4.0f : 0.0f), 
                                    // thisFace[25] + coord.x, thisFace[26] + coord.y + blockShift, thisFace[27] + coord.z, thisFace[28]+blockLightVal,
                                    // thisFace[29]+ (skyBlocked ? -4.0f : 0.0f)
                                });

                                    tuvs.insert(tuvs.end() , {
                                        thisTex[0].bl.x - textureWidth, thisTex[0].bl.y,  thisTex[0].br.x, thisTex[0].br.y,
                                        thisTex[0].br.x, thisTex[0].br.y,              thisTex[0].br.x, thisTex[0].br.y,
                                        thisTex[0].tr.x, thisTex[0].tr.y - textureWidth,  thisTex[0].br.x, thisTex[0].br.y,

                                        // thisTex[0].tr.x, thisTex[0].tr.y,
                                        // thisTex[0].tl.x, thisTex[0].tl.y,
                                        // thisTex[0].bl.x, thisTex[0].bl.y
                                    });
                                } else if(neighborIndex == BOTTOM) {

                                    tverts.insert(tverts.end(), {
                                    thisFace[0] + coord.x, thisFace[1] + coord.y, thisFace[2] + coord.z, thisFace[3]+blockLightVal,
                                    thisFace[4]+ (skyBlocked ? -4.0f : 0.0f), 
                                    thisFace[5] + coord.x, thisFace[6] + coord.y, thisFace[7] + coord.z, thisFace[8]+blockLightVal,
                                    thisFace[9]+ (skyBlocked ? -4.0f : 0.0f), 
                                    thisFace[10] + coord.x, thisFace[11] + coord.y, thisFace[12] + coord.z, thisFace[13]+blockLightVal,
                                    thisFace[14]+ (skyBlocked ? -4.0f : 0.0f), 

                                    // thisFace[15] + coord.x, thisFace[16] + coord.y, thisFace[17] + coord.z, thisFace[18]+blockLightVal,
                                    // thisFace[19]+ (skyBlocked ? -4.0f : 0.0f), 
                                    // thisFace[20] + coord.x, thisFace[21] + coord.y, thisFace[22] + coord.z, thisFace[23]+blockLightVal,
                                    // thisFace[24]+ (skyBlocked ? -4.0f : 0.0f), 
                                    // thisFace[25] + coord.x, thisFace[26] + coord.y, thisFace[27] + coord.z, thisFace[28]+blockLightVal,
                                    // thisFace[29]+ (skyBlocked ? -4.0f : 0.0f)
                                    });

                                    tuvs.insert(tuvs.end() , {
                                        thisTex[2].bl.x - textureWidth, thisTex[2].bl.y,   thisTex[2].br.x, thisTex[2].br.y,
                                        thisTex[2].br.x, thisTex[2].br.y,               thisTex[2].br.x, thisTex[2].br.y,
                                        thisTex[2].tr.x, thisTex[2].tr.y - textureWidth,   thisTex[2].br.x, thisTex[2].br.y,

                                        // thisTex[2].tr.x, thisTex[2].tr.y,
                                        // thisTex[2].tl.x, thisTex[2].tl.y,
                                        // thisTex[2].bl.x, thisTex[2].bl.y
                                    });
                                } else {

                                    tverts.insert(tverts.end(), {
                                    thisFace[0] + coord.x, thisFace[1] + coord.y, thisFace[2] + coord.z, thisFace[3]+blockLightVal,
                                    thisFace[4]+ (skyBlocked ? -4.0f : 0.0f), 
                                    thisFace[5] + coord.x, thisFace[6] + coord.y, thisFace[7] + coord.z, thisFace[8]+blockLightVal,
                                    thisFace[9]+ (skyBlocked ? -4.0f : 0.0f), 
                                    thisFace[10] + coord.x, thisFace[11] + coord.y  + blockShift, thisFace[12] + coord.z, thisFace[13]+blockLightVal,
                                    thisFace[14]+ (skyBlocked ? -4.0f : 0.0f), 

                                    // thisFace[15] + coord.x, thisFace[16] + coord.y  + blockShift, thisFace[17] + coord.z, thisFace[18]+blockLightVal,
                                    // thisFace[19]+ (skyBlocked ? -4.0f : 0.0f), 
                                    // thisFace[20] + coord.x, thisFace[21] + coord.y  + blockShift, thisFace[22] + coord.z, thisFace[23]+blockLightVal,
                                    // thisFace[24]+ (skyBlocked ? -4.0f : 0.0f), 
                                    // thisFace[25] + coord.x, thisFace[26] + coord.y, thisFace[27] + coord.z, thisFace[28]+blockLightVal,
                                    // thisFace[29]+ (skyBlocked ? -4.0f : 0.0f)
                                    });


                                    tuvs.insert(tuvs.end() , {
                                        thisTex[1].bl.x - textureWidth, thisTex[1].bl.y  - bottomTextureShift,  thisTex[1].br.x, thisTex[1].br.y,
                                        thisTex[1].br.x, thisTex[1].br.y  - bottomTextureShift,              thisTex[1].br.x, thisTex[1].br.y,
                                        thisTex[1].tr.x, thisTex[1].tr.y - textureWidth,                        thisTex[1].br.x, thisTex[1].br.y,

                                        // thisTex[1].tr.x, thisTex[1].tr.y,
                                        // thisTex[1].tl.x, thisTex[1].tl.y,
                                        // thisTex[1].bl.x, thisTex[1].bl.y - bottomTextureShift
                                    });
                                }
                            }
                            neighborIndex++;
                        }

                    }else {


                        for(BlockCoord &neigh : BlockInfo::neighbors) {
                            uint32_t neighblockcombined = blockAtMemo(coord + neigh, memo);
                            uint32_t neighblock = neighblockcombined & BlockInfo::BLOCK_ID_BITS;
                            bool neighborTransparent = std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), neighblock) != BlockInfo::transparents.end() ||
                            std::find(BlockInfo::semiTransparents.begin(), BlockInfo::semiTransparents.end(), neighblock) != BlockInfo::semiTransparents.end();
                            bool solidNeighboringTransparent = 
                            (neighborTransparent && 
                            std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), block) == BlockInfo::transparents.end());
                            if(neighblock == 0 || solidNeighboringTransparent) {
                                BlockCoord lightCubeHere = coord + neigh;
                                //std::cout << lightCubeHere.x << " " << lightCubeHere.y << " " << lightCubeHere.z << "\n";
                                bool skyBlocked = false;
                                int yTest = lightCubeHere.y;
                                if(neighborIndex != BOTTOM) {
                                    while(yTest < chunkHeight) {
                                        yTest++;
                                        BlockCoord test(lightCubeHere.x, yTest, lightCubeHere.z);
                                        uint32_t blockHere = blockAtMemo(test, memo);
                                        uint32_t blockIDHere = blockHere & BlockInfo::BLOCK_ID_BITS;
                                        if(blockIDHere != 0 && std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), blockIDHere) == BlockInfo::transparents.end()) {
                                            skyBlocked = true;
                                            break;
                                        }
                                    }
                                }

                                float blockLightVal = 0.0f;
                                auto segIt = lightMap.find(lightCubeHere);
                                if(segIt != lightMap.end()) {
                                    for(LightRay& ray : segIt->second.rays) {
                                        blockLightVal = std::min(blockLightVal + ray.value, 16.0f);
                                    }
                                }

                                std::vector<float> &thisFace = faces[neighborIndex];

                                verts.insert(verts.end(), {
                                    thisFace[0] + coord.x, thisFace[1] + coord.y, thisFace[2] + coord.z, thisFace[3] + blockLightVal,
                                    thisFace[4]+ (skyBlocked ? -4.0f : 0.0f), 
                                    thisFace[5] + coord.x, thisFace[6] + coord.y, thisFace[7] + coord.z, thisFace[8] + blockLightVal,
                                    thisFace[9]+ (skyBlocked ? -4.0f : 0.0f), 
                                    thisFace[10] + coord.x, thisFace[11] + coord.y, thisFace[12] + coord.z, thisFace[13] + blockLightVal,
                                    thisFace[14]+ (skyBlocked ? -4.0f : 0.0f), 

                                    // thisFace[15] + coord.x, thisFace[16] + coord.y, thisFace[17] + coord.z, thisFace[18] + blockLightVal,
                                    // thisFace[19]+ (skyBlocked ? -4.0f : 0.0f), 
                                    // thisFace[20] + coord.x, thisFace[21] + coord.y, thisFace[22] + coord.z, thisFace[23] + blockLightVal,
                                    // thisFace[24]+ (skyBlocked ? -4.0f : 0.0f), 
                                    // thisFace[25] + coord.x, thisFace[26] + coord.y, thisFace[27] + coord.z, thisFace[28] + blockLightVal,
                                    // thisFace[29]+ (skyBlocked ? -4.0f : 0.0f)
                                });

                                std::vector<TextureFace> &thisTex = BlockInfo::texs[block];

                                if(neighborIndex == TOP) {
                                    uvs.insert(uvs.end() , {
                                        thisTex[0].bl.x - textureWidth, thisTex[0].bl.y,   thisTex[0].br.x, thisTex[0].br.y,
                                        thisTex[0].br.x, thisTex[0].br.y,                thisTex[0].br.x, thisTex[0].br.y,
                                        thisTex[0].tr.x, thisTex[0].tr.y - textureWidth,   thisTex[0].br.x, thisTex[0].br.y,

                                        // thisTex[0].tr.x, thisTex[0].tr.y,
                                        // thisTex[0].tl.x, thisTex[0].tl.y,
                                        // thisTex[0].bl.x, thisTex[0].bl.y
                                    });
                                } else if(neighborIndex == BOTTOM) {
                                    uvs.insert(uvs.end() , {
                                        thisTex[2].bl.x - textureWidth, thisTex[2].bl.y,   thisTex[2].br.x, thisTex[2].br.y,
                                        thisTex[2].br.x, thisTex[2].br.y,              thisTex[2].br.x, thisTex[2].br.y,
                                        thisTex[2].tr.x, thisTex[2].tr.y - textureWidth,   thisTex[2].br.x, thisTex[2].br.y,

                                        // thisTex[2].tr.x, thisTex[2].tr.y,
                                        // thisTex[2].tl.x, thisTex[2].tl.y,
                                        // thisTex[2].bl.x, thisTex[2].bl.y
                                    });
                                } else {
                                    uvs.insert(uvs.end() , {
                                        thisTex[1].bl.x - textureWidth, thisTex[1].bl.y,   thisTex[1].br.x, thisTex[1].br.y,
                                        thisTex[1].br.x, thisTex[1].br.y,               thisTex[1].br.x, thisTex[1].br.y,
                                        thisTex[1].tr.x, thisTex[1].tr.y - textureWidth,   thisTex[1].br.x, thisTex[1].br.y,

                                        // thisTex[1].tr.x, thisTex[1].tr.y,
                                        // thisTex[1].tl.x, thisTex[1].tl.y,
                                        // thisTex[1].bl.x, thisTex[1].bl.y
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

    float elapsed = glfwGetTime() - time;
    timeChunkMeshing += elapsed;
    numberOfSamples += 1;

}

void VoxelWorld::depropogateLightOriginIteratively(BlockCoord origin, std::set<BlockChunk*> *imp, std::unordered_map<BlockCoord,LightSegment,IntTupHash>& lightMap) {
    std::stack<BlockCoord> stack;

    // Start by pushing the origin
    stack.push(origin);

    while (!stack.empty()) {
        BlockCoord spot = stack.top();
        stack.pop();

        ChunkCoord chunkCoordOfOrigin(
            std::floor(static_cast<float>(origin.x)/chunkWidth),
            std::floor(static_cast<float>(origin.z)/chunkWidth)
        );

        ChunkCoord chunkCoordHere(
            std::floor(static_cast<float>(spot.x)/chunkWidth),
            std::floor(static_cast<float>(spot.z)/chunkWidth)
        );

        if(chunkCoordOfOrigin != chunkCoordHere) {
            auto chunkIt = takenCareOfChunkSpots.find(chunkCoordHere);
            if(chunkIt != takenCareOfChunkSpots.end()) {
                imp->insert(chunkIt->second);
            }
        }

        auto segIt = lightMap.find(spot);
        if(segIt != lightMap.end()) {
            auto rayIt = std::find_if(segIt->second.rays.begin(), segIt->second.rays.end(), [origin](LightRay& ray){
                return ray.origin == origin;
            });

            if(rayIt != segIt->second.rays.end()) {
                std::vector<int> directions = rayIt->directions;
                segIt->second.rays.erase(rayIt);
                for(int direction : directions) {
                    // Add neighboring spots to the stack for further processing
                    stack.push(spot + BlockInfo::neighbors[direction]);
                }
            }
        }
    }
}




void VoxelWorld::depropogateLightOrigin(BlockCoord spot, BlockCoord origin, std::set<BlockChunk*> *imp, std::unordered_map<
            BlockCoord,
            LightSegment,
            IntTupHash
    >& lightMap  ) {

    ChunkCoord chunkCoordOfOrigin(
        std::floor(static_cast<float>(origin.x)/chunkWidth),
        std::floor(static_cast<float>(origin.z)/chunkWidth)
    );

    ChunkCoord chunkCoordHere(
        std::floor(static_cast<float>(spot.x)/chunkWidth),
        std::floor(static_cast<float>(spot.z)/chunkWidth)
    );

    if(chunkCoordOfOrigin != chunkCoordHere) {
        auto chunkIt = takenCareOfChunkSpots.find(chunkCoordHere);
        if(chunkIt != takenCareOfChunkSpots.end()) {
            imp->insert(chunkIt->second);
        }
    }

    auto segIt = lightMap.find(spot);
    if(segIt != lightMap.end()) {


        auto rayIt = std::find_if(segIt->second.rays.begin(), segIt->second.rays.end(), [origin](LightRay& ray){
            return ray.origin == origin;
        });

        if(rayIt != segIt->second.rays.end()) {
            std::vector<int> directions = rayIt->directions;
            segIt->second.rays.erase(rayIt);
            for(int direction : directions) {
                depropogateLightOrigin(spot + BlockInfo::neighbors[direction], origin, imp, lightMap);
            }
        }



    }

}

struct spotForQueue {
    int value;
    BlockCoord spot;
};

void VoxelWorld::propogateLightOriginIteratively(BlockCoord spot, BlockCoord origin, int value, std::set<BlockChunk*> *imp, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo, std::unordered_map<BlockCoord, LightSegment, IntTupHash> &lightMap) {
    std::stack<spotForQueue> stack;
    std::unordered_map<BlockCoord, bool, IntTupHash> visited;

    stack.push(spotForQueue{value, spot});
    visited[spot] = true;

    while (!stack.empty()) {
        spotForQueue n = stack.top();
        stack.pop();

        uint32_t blockBitsHere = blockAtMemo(n.spot, memo);
        uint32_t blockIDHere = blockBitsHere & BlockInfo::BLOCK_ID_BITS;
        bool goingHere = (blockIDHere == 0 || 
        std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), blockIDHere) != BlockInfo::transparents.end() ||
        std::find(BlockInfo::semiTransparents.begin(), BlockInfo::semiTransparents.end(), blockIDHere) != BlockInfo::semiTransparents.end() ||
        n.spot == origin);

        if (goingHere) {

            ChunkCoord chunkCoordOfOrigin(
                std::floor(static_cast<float>(origin.x)/chunkWidth),
                std::floor(static_cast<float>(origin.z)/chunkWidth)
            );

            ChunkCoord chunkCoordHere(
                std::floor(static_cast<float>(n.spot.x)/chunkWidth),
                std::floor(static_cast<float>(n.spot.z)/chunkWidth)
            );

            if(chunkCoordOfOrigin != chunkCoordHere) {
                auto chunkIt = takenCareOfChunkSpots.find(chunkCoordHere);
                if(chunkIt != takenCareOfChunkSpots.end()) {
                    imp->insert(chunkIt->second);
                }
            }



            auto segIt = lightMap.find(n.spot);
            if (segIt == lightMap.end()) {
                segIt = lightMap.insert({n.spot, LightSegment()}).first;
            }

            auto rayIt = std::find_if(segIt->second.rays.begin(), segIt->second.rays.end(), [origin](LightRay& ray) {
                return ray.origin == origin;
            });

            if (rayIt == segIt->second.rays.end()) {
                segIt->second.rays.emplace_back();
                rayIt = std::prev(segIt->second.rays.end());

                rayIt->value = n.value;
                rayIt->origin = origin;
            } else if (rayIt->value < n.value) {
                rayIt->value = n.value;
            }

            if (n.value > 1) {
                for (int i = 0; i < BlockInfo::neighbors.size(); ++i) {
                    BlockCoord next = n.spot + BlockInfo::neighbors[i];

                    // Find or create a ray for the neighbor
                    auto nextSegIt = lightMap.find(next);
                    if (nextSegIt == lightMap.end()) {
                        nextSegIt = lightMap.insert({next, LightSegment()}).first;
                    }
                    auto nextRayIt = std::find_if(nextSegIt->second.rays.begin(), nextSegIt->second.rays.end(), [origin](const LightRay& ray) {
                        return ray.origin == origin;
                    });
                    int nextValue = (nextRayIt != nextSegIt->second.rays.end()) ? nextRayIt->value : 0;

                    // Check if the neighbor's value is less than our value - 1
                    if (visited.find(next) == visited.end() || nextValue < n.value - 1) {
                        stack.push(spotForQueue{n.value - 1, next});
                        visited[next] = true;

                        if (nextRayIt == nextSegIt->second.rays.end()) {
                            nextSegIt->second.rays.emplace_back(); // Add new ray
                            nextRayIt = std::prev(nextSegIt->second.rays.end());
                            nextRayIt->value = n.value - 1;
                            nextRayIt->origin = origin;
                        }

                        if (std::find(rayIt->directions.begin(), rayIt->directions.end(), i) == rayIt->directions.end()) {
                            rayIt->directions.push_back(i);
                        }
                    }
                }
            }
        }
    }
}

void VoxelWorld::propogateLightOrigin(BlockCoord spot, BlockCoord origin, int value, std::set<BlockChunk*> *imp, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo, std::unordered_map<
            BlockCoord,
            LightSegment,
            IntTupHash
    >  &lightMap) {
    if(value > 0) {

        ChunkCoord chunkCoordOfOrigin(
            std::floor(static_cast<float>(origin.x)/chunkWidth),
            std::floor(static_cast<float>(origin.z)/chunkWidth)
        );

        ChunkCoord chunkCoordHere(
            std::floor(static_cast<float>(spot.x)/chunkWidth),
            std::floor(static_cast<float>(spot.z)/chunkWidth)
        );

       if(chunkCoordOfOrigin != chunkCoordHere) {
            auto chunkIt = takenCareOfChunkSpots.find(chunkCoordHere);
            if(chunkIt != takenCareOfChunkSpots.end()) {
                imp->insert(chunkIt->second);
            }
        }

        auto segIt = lightMap.find(spot);
        if(segIt == lightMap.end()) {
            lightMap.insert_or_assign(spot, LightSegment());
        }
        segIt = lightMap.find(spot);

        auto rayIt = std::find_if(segIt->second.rays.begin(), segIt->second.rays.end(), [origin](LightRay& ray){
            return ray.origin == origin;
        });

        //if theres not already a ray of this origin here
        if(rayIt == segIt->second.rays.end()) {

            int neighborIndex = 0;
            LightRay thisRay;
            thisRay.value = value;
            thisRay.origin = origin;
            //figure out the directions its going
            for(BlockCoord& neigh : BlockInfo::neighbors) {
                uint32_t blockBitsHere = blockAtMemo(spot + neigh, memo);
                uint32_t blockIDHere = blockBitsHere & BlockInfo::BLOCK_ID_BITS;
                bool goingHere = (blockIDHere == 0 || 
                std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), blockIDHere) != BlockInfo::transparents.end() ||
                std::find(BlockInfo::semiTransparents.begin(), BlockInfo::semiTransparents.end(), blockIDHere) != BlockInfo::semiTransparents.end());
                if(goingHere) {
                    thisRay.directions.push_back(neighborIndex);
                }
                neighborIndex++;
            }

            segIt->second.rays.push_back(thisRay);
            for(int dir : thisRay.directions) {

                propogateLightOrigin(spot + BlockInfo::neighbors[dir], origin, value - 1, imp, memo, lightMap);

            }

        } else { //if there is a ray of this origin here, then if it is less than our value, bring it up to it, and propogate on.
            if(rayIt->value < value) {
                rayIt->value = value;

            
                int neighborIndex = 0;
                //figure out the directions its going
                for(BlockCoord& neigh : BlockInfo::neighbors) {
                    uint32_t blockBitsHere = blockAtMemo(spot + neigh, memo);
                    uint32_t blockIDHere = blockBitsHere & BlockInfo::BLOCK_ID_BITS;
                    bool goingHere = (blockIDHere == 0 || 
                    std::find(BlockInfo::transparents.begin(), BlockInfo::transparents.end(), blockIDHere) != BlockInfo::transparents.end() ||
                    std::find(BlockInfo::semiTransparents.begin(), BlockInfo::semiTransparents.end(), blockIDHere) != BlockInfo::semiTransparents.end());
                    if(goingHere) {
                        if(std::find(rayIt->directions.begin(), rayIt->directions.end(), neighborIndex) == rayIt->directions.end()) {
                            rayIt->directions.push_back(neighborIndex);
                        }
                    }
                    neighborIndex++;
                }

                for(int dir : rayIt->directions) {

                    propogateLightOrigin(spot + BlockInfo::neighbors[dir], origin, value - 1, imp, memo, lightMap);

                }
            }

        }
    }
}



void VoxelWorld::lightPassOnChunk(ChunkCoord chunkCoord, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo) {

    if(hasHadInitialLightPass.find(chunkCoord) == hasHadInitialLightPass.end()) {
        hasHadInitialLightPass.insert_or_assign(chunkCoord, true);
    }



    std::set<BlockChunk*> implicatedChunks;

    std::unordered_set<BlockCoord, IntTupHash> lightSources;


    auto chunkIt = userDataMap.find(chunkCoord);

    if(chunkIt != userDataMap.end()) {


        //remove and depropogate light sources
        for(int x = 0; x < chunkWidth; ++x) {
            for(int z = 0; z < chunkWidth; ++z) {
                for(int y = 0; y < chunkHeight; ++y) {
                    BlockCoord coord(chunkCoord.x * chunkWidth + x, y, chunkCoord.z * chunkWidth + z);
                    auto lightIt = lightMap.find(coord);
                    if(lightIt != lightMap.end()) {
                        for(LightRay& ray : lightIt->second.rays) {


                            ChunkCoord chunkCoordOfOrigin(
                                std::floor(static_cast<float>(ray.origin.x)/chunkWidth),
                                std::floor(static_cast<float>(ray.origin.z)/chunkWidth)
                            );
                            //std::cout << "Checking " << chunkCoordOfOrigin.x << " " << chunkCoordOfOrigin.z << "\n";
                            if(chunkCoordOfOrigin == chunkCoord) {
                                //std::cout << "We should be removing origin " << ray.origin.x << " " << ray.origin.y << " " << ray.origin.z << "\n";
                                BlockCoord originWeRemoving = ray.origin;

                                depropogateLightOriginIteratively(originWeRemoving, &implicatedChunks, lightMap);
                                if(blockAtMemo(originWeRemoving, memo) == 12) {
                                    lightSources.insert(originWeRemoving);
                                }
                            }

                            
                        }
                        
                    }
                    if(blockAtMemo(coord, memo) == 12) {
                        lightSources.insert(coord);
                    }
                }
            }
        }

        for(BlockCoord source : lightSources) {
            propogateLightOriginIteratively(source, source, 8, &implicatedChunks, memo, lightMap);
        }


    }

    for(BlockChunk *pointer : implicatedChunks) {
        while(!deferredChunkQueue.push(pointer)) {

        }
    }
}

















void VoxelWorld::generateChunk(ChunkCoord chunkcoord, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo) {

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

    BlockCoord(-1,-2,-2),
    BlockCoord(-1,-2,2),

    BlockCoord(0,-2,-2),
    BlockCoord(0,-2,2),

    BlockCoord(1,-2,-2),
    BlockCoord(1,-2,2),

    BlockCoord(0,1,0),
    BlockCoord(-1,1,0),
    BlockCoord(1,1,0),
    BlockCoord(0,1,1),
    BlockCoord(0,1,-1),

    // Additional spots for a taller shape
    BlockCoord(-1,-3,0),
    BlockCoord(1,-3,0),
    BlockCoord(0,-3,1),
    BlockCoord(0,-3,-1),

    BlockCoord(-1,-3,1),
    BlockCoord(1,-3,1),
    BlockCoord(1,-3,-1),
    BlockCoord(-1,-3,-1),

    BlockCoord(-1,-4,0),
    BlockCoord(1,-4,0),
    BlockCoord(0,-4,1),
    BlockCoord(0,-4,-1),

    BlockCoord(-1,-4,1),
    BlockCoord(1,-4,1),
    BlockCoord(1,-4,-1),
    BlockCoord(-1,-4,-1),

    BlockCoord(-1,-5,0),
    BlockCoord(1,-5,0),
    BlockCoord(0,-5,1),
    BlockCoord(0,-5,-1),

    BlockCoord(-1,-6,0),
    BlockCoord(1,-6,0),
    BlockCoord(0,-6,1),
    BlockCoord(0,-6,-1),

    BlockCoord(-1,-7,0),
    BlockCoord(1,-7,0),
    BlockCoord(0,-7,1),
    BlockCoord(0,-7,-1),

    BlockCoord(0,-8,0),
    BlockCoord(-1,-8,0),
    BlockCoord(1,-8,0),
    BlockCoord(0,-8,1),
    BlockCoord(0,-8,-1),

    BlockCoord(0,2,0),
    BlockCoord(-1,2,0),
    BlockCoord(1,2,0),
    BlockCoord(0,2,1),
    BlockCoord(0,2,-1),

    BlockCoord(0,3,0),
    BlockCoord(-1,3,0),
    BlockCoord(1,3,0),
    BlockCoord(0,3,1),
    BlockCoord(0,3,-1),

    // Top layer
    BlockCoord(0, 4, 0),
    BlockCoord(-1, 4, 0),
    BlockCoord(1, 4, 0),
    BlockCoord(0, 4, 1),
    BlockCoord(0, 4, -1),

    // Second layer
    BlockCoord(-2, 3, 0),
    BlockCoord(2, 3, 0),
    BlockCoord(0, 3, 2),
    BlockCoord(0, 3, -2),

    // Third layer
    BlockCoord(-2, 2, 0),
    BlockCoord(2, 2, 0),
    BlockCoord(0, 2, 2),
    BlockCoord(0, 2, -2),
    BlockCoord(-2, 2, 1),
    BlockCoord(2, 2, 1),
    BlockCoord(-2, 2, -1),
    BlockCoord(2, 2, -1),
    BlockCoord(-1, 2, 2),
    BlockCoord(1, 2, 2),
    BlockCoord(-1, 2, -2),
    BlockCoord(1, 2, -2),

    // Middle layer
    BlockCoord(-3, 1, 0),
    BlockCoord(3, 1, 0),
    BlockCoord(0, 1, 3),
    BlockCoord(0, 1, -3),
    BlockCoord(-2, 1, 2),
    BlockCoord(2, 1, 2),
    BlockCoord(-2, 1, -2),
    BlockCoord(2, 1, -2),
    BlockCoord(-1, 1, 3),
    BlockCoord(1, 1, 3),
    BlockCoord(-1, 1, -3),
    BlockCoord(1, 1, -3),

    // Lower layers
    BlockCoord(-3, 0, 0),
    BlockCoord(3, 0, 0),
    BlockCoord(0, 0, 3),
    BlockCoord(0, 0, -3),

    BlockCoord(-3, -1, 0),
    BlockCoord(3, -1, 0),
    BlockCoord(0, -1, 3),
    BlockCoord(0, -1, -3),
    
    BlockCoord(-2, -1, 0),
BlockCoord(2, -1, 0),
BlockCoord(0, -1, 2),
BlockCoord(0, -1, -2),

BlockCoord(-2, 0, 0),
BlockCoord(2, 0, 0),
BlockCoord(0, 0, 2),
BlockCoord(0, 0, -2),

BlockCoord(-2, 1, 0),
BlockCoord(2, 1, 0),
BlockCoord(0, 1, 2),
BlockCoord(0, 1, -2),
};






    std::set<BlockChunk*> implicatedChunks;

    for(int x = 0; x < chunkWidth; ++x) {
        for(int z = 0; z < chunkWidth; ++z) {
            for(int y = 0; y < chunkHeight; ++y) {
                BlockCoord coord(chunkcoord.x * chunkWidth + x, y, chunkcoord.z * chunkWidth + z);

                //Trees
                if(blockAtMemo(coord, memo) == 3) {
                    int ran = randsmall();
                    if(ran > 126) {
                        BlockCoord here = coord;
                        int height = 5 + static_cast<int>(4.0f*rando());
                        for(int i = 0; i < height; ++i) {
                            
                            here += BlockCoord(0,1,0);
                            bool doingIt = true;
                            udmMutex.lock();  
            
                            auto chunkIt = userDataMap.find(chunkcoord);
                            if(chunkIt != userDataMap.end()) {
                                if(chunkIt->second.find(here) != chunkIt->second.end()) {
                                    doingIt = false;
                                }
                            }
                            udmMutex.unlock();     
                            if(doingIt) {
                                nudmMutex.lock();
                                nonUserDataMap.insert_or_assign(here, 6);
                                nudmMutex.unlock();
                                auto memoIt = memo.find(here);
                                if(memoIt != memo.end()){
                                    memo.erase(here);
                                }
                            }
                        }
                        for(BlockCoord& coor : leafSpots) {
                            BlockCoord coorHere = here + coor;
                            bool doingIt = true;
                            udmMutex.lock();  
            
                            auto chunkIt = userDataMap.find(chunkcoord);
                            if(chunkIt != userDataMap.end()) {
                                if(chunkIt->second.find(coorHere) != chunkIt->second.end()) {
                                    doingIt = false;
                                }
                            }
                            udmMutex.unlock();   
                            if(doingIt) {
                                ChunkCoord chunkHere(
                                    std::floor(static_cast<float>(coorHere.x)/chunkWidth),
                                    std::floor(static_cast<float>(coorHere.z)/chunkWidth)
                                );
                                nudmMutex.lock();
                                if(nonUserDataMap.find(coorHere) == nonUserDataMap.end()) {
                                    nonUserDataMap.insert_or_assign(coorHere, 7);
                                    auto memoIt = memo.find(coorHere);
                                    if(memoIt != memo.end()){
                                        memo.erase(coorHere);
                                    }
                                    auto chunkIt = takenCareOfChunkSpots.find(chunkHere);
                                    if(chunkIt != takenCareOfChunkSpots.end()) {
                                        implicatedChunks.insert(chunkIt->second);
                                    }
                                }
                                nudmMutex.unlock();
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

    
    ChunkCoord chunkcoord(
        static_cast<int>(std::floor(static_cast<float>(coord.x)/chunkWidth)), 
        static_cast<int>(std::floor(static_cast<float>(coord.z)/chunkWidth))
    );
udmMutex.lock();  
               
    auto chunkit = userDataMap.find(chunkcoord);
    if(chunkit != userDataMap.end()) {
        auto blockit = chunkit->second.find(coord);
        if(blockit != chunkit->second.end()) {

            uint32_t b = blockit->second;
            udmMutex.unlock(); 
            return b;
             
        }
    }

 udmMutex.unlock(); 
    if((*currentNoiseFunction)(coord.x, coord.y, coord.z) > 10) {
        if((*currentNoiseFunction)(coord.x, coord.y+10, coord.z) > 10) {
            return 5;
        }
        if(coord.y > waterLevel + 2 || (*currentNoiseFunction)(coord.x, coord.y+5, coord.z) > 10) {
            if((*currentNoiseFunction)(coord.x, coord.y+1, coord.z) < 10) {
                        
                        return 3;
                        
                    }
            return 4;
        } else {
            return 1;
        }
    }
    nudmMutex.lock();
    auto blockit = nonUserDataMap.find(coord);
    if(blockit != nonUserDataMap.end()) {
        uint32_t block = blockit->second;
        nudmMutex.unlock();
        return block;
    }
    nudmMutex.unlock();
    if(coord.y < waterLevel) {
        return 2;
    }
    return 0;
}

uint32_t VoxelWorld::blockAtMemo(BlockCoord coord, std::unordered_map<BlockCoord, uint32_t, IntTupHash>& memo) {
    auto blockIt = memo.find(coord);
    if(blockIt != memo.end()) {
        return blockIt->second;
    }

    uint32_t res = blockAt(coord);
    memo.insert_or_assign(coord, res);
    return res;
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
        outputFile << "version " << worldGenVersion << " " << seed << '\n';
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

void VoxelWorld::deleteFolder(std::string pathString) {
    std::filesystem::path path(pathString);

    if (!std::filesystem::exists(path)) {
        std::cerr << "Path does not exist: " << pathString << std::endl;
        return;
    }

    // Recursively delete all contents
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (std::filesystem::is_directory(entry.path())) {
            deleteFolder(entry.path().string());
        } else {
            std::filesystem::remove(entry.path());
        }
    }

    // Delete the directory itself
    std::filesystem::remove(path);
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
                std::istringstream linestream(line);
                std::vector<std::string> words;
                std::string word;

                while(linestream >> word) {
                    words.push_back(word);
                }

                if(words.at(0) == "version") {
                    worldGenVersion = std::stoi(words.at(1));
                    if(worldGenVersion == 1) {
                        waterLevel = 20;
                    } else {
                        waterLevel = 40;
                    }
                    seed = static_cast<unsigned int>(std::stoi(words.at(2)));
                    getOffsetFromSeed();
                } else {
                    worldGenVersion = 1;
                    waterLevel = 20;
                    seed = static_cast<unsigned int>(std::stoi(words.at(0)));
                }
                currentNoiseFunction = &(worldGenFunctions.at(worldGenVersion));
                std::cout << "Worldgen version is " << worldGenVersion << "\n";

                
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

int VoxelWorld::checkVersionOfSave(const char *path) {
    std::string worldPath(path);
    worldPath += "/world.save";
    if(!std::filesystem::exists(worldPath)) {
        return 2;
    }
    std::ifstream file(worldPath);
    if(file.is_open()) {
        std::string line;

        std::getline(file, line);

                std::istringstream linestream(line);
                std::vector<std::string> words;
                std::string word;

                while(linestream >> word) {
                    words.push_back(word);
                }

                if(words.at(0) == "version") {
                    return std::stoi(words.at(1));
                } else {
                    return 1;
                }

        file.close();
    } else {
        throw std::exception("Could not open world file.");
    }
}