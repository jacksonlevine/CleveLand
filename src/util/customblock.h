#ifndef CUSTOMBLOCK_H
#define CUSTOMBLOCK_H

#include <vector>

struct CustomBlock {

    unsigned int blockID;
    std::vector<float> verts;
    std::vector<float> uvs;

};

#endif