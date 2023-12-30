#ifndef NUGGO_H
#define NUGGO_H

#include <vector>
#include <entt/entt.hpp>

struct Nuggo {
    std::vector<float> verts;
    std::vector<float> uvs;
    entt::entity me;
};

#endif