#ifndef BLOCKINFO_H
#define BLOCKINFO_H

#include "textureface.h"
#include <vector>
#include "customblock.h"
#include "vertexutils.h"

class BlockInfo {
public:

    inline static std::vector<float> breakTimes = {
        0.0f,
        0.3f,//sand
        1.0f,//water
        0.6f,//grass
        0.6f,//dirt
        1.2f,//rock
        1.0f,//wood
        0.25f,//leaves
        0.5f,//glass
        1.0f,//smooth stone
        0.5f,//planks
        0.5f,//door bottom
        0.5f,//light
        1.0f,//chest
        0.5f, //ladder

        0.3f,//red
        1.3f,//brick
        1.3f,//moss
        1.3f, //cool stone

        0.7f, //post
        0.3f,//torch,
        1.0f, //sign,
        1.0f, //bigsign
        1.0f, //stair
    };
    inline static std::vector<std::vector<TextureFace>> texs = {
        {TextureFace(0,0),//top
        TextureFace(0,0),//sides
        TextureFace(0,0),//bottom
        }, 
        {TextureFace(1,0),//sand1
        TextureFace(1,0),
        TextureFace(1,0),
        }, 
        {TextureFace(2,0),//water2
        TextureFace(2,0),
        TextureFace(2,0),
        }, 
        {TextureFace(3,1),//grass3
        TextureFace(3,0),
        TextureFace(4,0),
        }, 
        {TextureFace(4,0),//dirt4
        TextureFace(4,0),
        TextureFace(4,0),
        },
        {TextureFace(5,0),//rock5
        TextureFace(5,0),
        TextureFace(5,0),
        },
        {TextureFace(6,1),//wood6
        TextureFace(6,0),
        TextureFace(6,1),
        },
        {TextureFace(7,0),//leaves7
        TextureFace(7,0),
        TextureFace(7,0),
        },
        {TextureFace(8,0),//glass8
        TextureFace(8,0),
        TextureFace(8,0),
        },
        {TextureFace(9,0),//smooth stone 9
        TextureFace(9,0),
        TextureFace(9,0),
        },
        {TextureFace(10,0),//planks 10
        TextureFace(10,0),
        TextureFace(10,0),
        },
        {TextureFace(11,0),//door bottom  11
        TextureFace(11,0),
        TextureFace(11,0),
        }, 
        {TextureFace(12,1),//light 12
        TextureFace(12,1),
        TextureFace(12,1),
        },
        {TextureFace(13,0),//chest 13
        TextureFace(13,0),
        TextureFace(13,0),
        },
        {TextureFace(0,1),//ladder 14
        TextureFace(0,1),
        TextureFace(0,1),
        },

        
        {TextureFace(0,2),//red block 15
        TextureFace(0,2),
        TextureFace(0,2),
        },

        
        {TextureFace(1,2),//brick block 16
        TextureFace(1,2),
        TextureFace(1,2),
        },

        
        {TextureFace(2,2),//lmossy stone 17
        TextureFace(2,2),
        TextureFace(2,2),
        },

        
        {TextureFace(3,2),//cool stone 18
        TextureFace(3,2),
        TextureFace(3,2),
        },

        
        {TextureFace(5,1),//post 19
        TextureFace(5,1),
        TextureFace(5,1),
        }

        ,

        
        {TextureFace(5,2),//torch 20
        TextureFace(5,2),
        TextureFace(5,2),
        }
        ,

        
        {TextureFace(10,1),//sign 21
        TextureFace(10,1),
        TextureFace(10,1),
        },

        
        {TextureFace(10,1),//bigsign 22
        TextureFace(10,1),
        TextureFace(10,1),
        },


        {TextureFace(7,1),//stair 23
        TextureFace(7,1),
        TextureFace(7,1),
        }
    };

    inline static std::vector<int> transparents = {
        2, 8,
    };

    inline static std::vector<int> semiTransparents = {
        7, 11, 14, 19, 20, 21, 22, 23
    };

    inline static std::vector<int> lights = {
        12, 20
    };

    inline static std::vector<int> placeInside = {
        0, 2
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