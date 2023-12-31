#ifndef BLOCKCHUNK_H
#define BLOCKCHUNK_H

#include "../util/chunkcoord.h"

struct BlockChunk {
    unsigned int nuggoPoolIndex;
    ChunkCoord position;
    bool used;
};

#endif