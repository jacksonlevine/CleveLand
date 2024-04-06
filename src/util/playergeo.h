#ifndef PLAYERGEO_H
#define PLAYERGEO_H

#include "glm/glm.hpp"

struct PlayerGeo {
    glm::vec3 lastPosition;
    glm::vec3 position;
    float timePosted;
    float rotation;
    float lastRotation;
};

#endif