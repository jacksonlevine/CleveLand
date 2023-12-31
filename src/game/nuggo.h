#ifndef NUGGO_H
#define NUGGO_H

#include <vector>
#include <entt/entt.hpp>

struct Nuggo {
    std::vector<float> verts;
    std::vector<float> uvs;

    std::vector<float> tverts;
    std::vector<float> tuvs;
    
    entt::entity me;
};

#endif