#ifndef BLOCKCHUNK_H
#define BLOCKCHUNK_H

#include "../util/chunkcoord.h"

struct BlockChunk {
    unsigned int nuggoPoolIndex;
    ChunkCoord position = ChunkCoord(999999,999999);
    bool used;
};

#endif