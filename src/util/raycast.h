#ifndef RAYCAST_H
#define RAYCAST_H

#include "chunkcoord.h"
#include <functional>
#include <glm/glm.hpp>
#include <iostream>

struct RayCastResult {
    bool hit = false;
    BlockCoord blockHit;
    std::vector<ChunkCoord> chunksToRebuild;
    glm::vec3 head;
};

RayCastResult rayCast(
    int chunkwidth, 
    glm::vec3 origin, 
    glm::vec3 dir, 
    std::function<bool(BlockCoord)> predicate, 
    bool includeSideChunks);

#endif