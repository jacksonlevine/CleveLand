#ifndef GEOMETRYSTORE_H
#define GEOMETRYSTORE_H

#include <vector>
#include <entt/entt.hpp>

struct GeometryStore {
    std::vector<float> verts;
    std::vector<float> uvs;

    std::vector<float> tverts;
    std::vector<float> tuvs;

    entt::entity me;
};

#endif