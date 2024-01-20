#ifndef LIGHTINFO_H
#define LIGHTINFO_H

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include "chunkcoord.h"
#include "blockinfo.h"

struct LightRay {
    std::vector<int> directions;
    int value;
    BlockCoord origin;
};

struct LightSegment {
    std::vector<LightRay> rays;
};



struct AmbientLightRay {
    std::vector<int> directions;
    int value;
    BlockCoord origin;
    bool isOrigin;
};

struct AmbientLightSegment {
    std::vector<AmbientLightRay> rays;
};



#endif