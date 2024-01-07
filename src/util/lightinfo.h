#ifndef LIGHTINFO_H
#define LIGHTINFO_H

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include "chunkcoord.h"

struct LightRay {
    glm::vec3 direction;
    float value;
    glm::vec3 origin;
};

struct LightSegment {
    std::vector<LightRay> rays;
};

struct LightMap {
    std::unordered_map<BlockCoord, LightSegment, IntTupHash> map;
    
};

#endif