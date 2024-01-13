#include "geometrystore.h"

GeometryStore::GeometryStore() {
    
}

GeometryStore::GeometryStore(const GeometryStore& other)
    : verts(other.verts),
      uvs(other.uvs),
      tverts(other.tverts),
      tuvs(other.tuvs),
      // Note: Mutex is not copied but newly initialized
      me(other.me)
{
    // The mutex is default-initialized, no need to copy it.
}

GeometryStore& GeometryStore::operator=(const GeometryStore& other) {
    if (this != &other) {
        verts = other.verts;
        uvs = other.uvs;
        tverts = other.tverts;
        tuvs = other.tuvs;
        me = other.me;
        // Mutex remains unique to each instance and is not assigned.
    }
    return *this;
}