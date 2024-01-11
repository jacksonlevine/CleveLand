#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position;
    float blockID;
    float timeCreated;
    float lifetime;
    glm::vec3 destination;
    float gravity;
    float floorAtDest;
};

#endif