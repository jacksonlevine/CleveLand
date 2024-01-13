#ifndef GEOMETRYSTORE_H
#define GEOMETRYSTORE_H

#include <vector>
#include <entt/entt.hpp>
#include <mutex>

struct GeometryStore {
    std::vector<float> verts;
    std::vector<float> uvs;

    std::vector<float> tverts;
    std::vector<float> tuvs;

    std::mutex myLock;

    entt::entity me;
    GeometryStore();
    GeometryStore(const GeometryStore& other);
    GeometryStore& operator=(const GeometryStore& other);
};

#endif