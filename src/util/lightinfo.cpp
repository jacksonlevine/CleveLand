#include "lightinfo.h"


float LightSegment::sum() {
    float res = 0.0f;
    for(LightRay& ray : rays) {
        res += ray.value;
    }
    return std::min(res, 16.0f);
}