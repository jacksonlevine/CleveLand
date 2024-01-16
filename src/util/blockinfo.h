#ifndef BLOCKINFO_H
#define BLOCKINFO_H

#include "textureface.h"
#include <vector>
#include "customblock.h"
#include "vertexutils.h"

class BlockInfo {
public:
    inline static std::vector<std::vector<TextureFace>> texs = {
        {TextureFace(0,0),//top
        TextureFace(0,0),//sides
        TextureFace(0,0),//bottom
        }, 
        {TextureFace(1,0),
        TextureFace(1,0),
        TextureFace(1,0),
        }, 
        {TextureFace(2,0),
        TextureFace(2,0),
        TextureFace(2,0),
        }, 
        {TextureFace(3,1),
        TextureFace(3,0),
        TextureFace(4,0),
        }, 
        {TextureFace(4,0),
        TextureFace(4,0),
        TextureFace(4,0),
        },
        {TextureFace(5,0),
        TextureFace(5,0),
        TextureFace(5,0),
        },
        {TextureFace(6,1),
        TextureFace(6,0),
        TextureFace(6,1),
        },
        {TextureFace(7,0),
        TextureFace(7,0),
        TextureFace(7,0),
        },
        {TextureFace(8,0),
        TextureFace(8,0),
        TextureFace(8,0),
        },
        {TextureFace(9,0),
        TextureFace(9,0),
        TextureFace(9,0),
        },
        {TextureFace(10,0),
        TextureFace(10,0),
        TextureFace(10,0),
        },
        {TextureFace(11,0),//door bottom
        TextureFace(11,0),
        TextureFace(11,0),
        },
        {TextureFace(12,1),//door bottom
        TextureFace(12,1),
        TextureFace(12,1),
        }
    };

    inline static std::vector<int> transparents = {
        2, 8,
    };

    inline static std::vector<int> semiTransparents = {
        7, 11
    };

    inline static std::vector<int> lights = {
        12
    };

    inline static uint32_t BLOCK_ID_BITS = 0b0000'0000'0000'0000'1111'1111'1111'1111;
    inline static uint32_t BLOCK_FLAG_BITS = 0b1111'1111'1111'1111'0000'0000'0000'0000;

    inline static uint32_t BLOCK_DIRECTION_BITS = 0b0000'0000'0000'0011'0000'0000'0000'0000;

    inline static uint32_t getDirectionBits(uint32_t input) {
        return (input & BLOCK_DIRECTION_BITS) >> 16;
    };

    inline static void setDirectionBits(uint32_t& input, uint32_t direction) {

        uint32_t bits = direction << 16;
        input |= bits;

    }; 

        //left right forward backward up down
    inline static std::vector<BlockCoord> neighbors = {
        BlockCoord(-1, 0, 0),
        BlockCoord(1, 0, 0),
        BlockCoord(0, 0, 1),
        BlockCoord(0, 0, -1),
        BlockCoord(0, 1, 0),
        BlockCoord(0, -1, 0),
    };

};

    enum Neighbors  {
        LEFT,
        RIGHT,
        FRONT,
        BACK,
        TOP,
        BOTTOM
    };



#endif