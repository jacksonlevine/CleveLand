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
        }
    };

    inline static std::vector<int> transparents = {
        2, 7, 8, 11
    };

    inline static std::vector<float> getDoorUVs(TextureFace face) {
        static TextureFace side(12, 0);
        std::vector<float> doorUVs = {

            face.bl.x, face.bl.y,
            face.br.x, face.br.y,
            face.tr.x, face.tr.y,

            face.tr.x, face.tr.y,
            face.tl.x, face.tl.y,
            face.bl.x, face.bl.y,//front
            
            
            side.br.x, side.br.y,
            side.tr.x, side.tr.y,
            side.br.x-(textureWidth/4), side.tr.y,
            
            side.br.x-(textureWidth/4), side.tr.y,
            side.br.x-(textureWidth/4), side.br.y,
            side.br.x, side.br.y,//left
            
            

            
            side.br.x, side.br.y,
            side.tr.x, side.tr.y,
            side.br.x-(textureWidth/4), side.tr.y,

            side.br.x-(textureWidth/4), side.tr.y,
            side.br.x-(textureWidth/4), side.br.y,
            side.br.x, side.br.y,//top
            
            

            side.br.x, side.br.y,
            side.tr.x, side.tr.y,
            side.br.x-(textureWidth/4), side.tr.y,

            side.br.x-(textureWidth/4), side.tr.y,
            side.br.x-(textureWidth/4), side.br.y,
            side.br.x, side.br.y,//right
            
            

            face.bl.x, face.bl.y,
            face.br.x, face.br.y,
            face.tr.x, face.tr.y,

            face.tr.x, face.tr.y,
            face.tl.x, face.tl.y,
            face.bl.x, face.bl.y,//back
            
        };
        return doorUVs;
    }

    inline static std::vector<float> baseDoorModel = {
        { //player is minus z
                -0.5f, -0.5f, -0.5f,0.0f, 16.0f,//front
                +0.5f, -0.5f, -0.5f,0.0f, 16.0f,
                +0.5f, +0.5f, -0.5f,0.0f, 16.0f,

                +0.5f, +0.5f, -0.5f,0.0f, 16.0f,
                -0.5f, +0.5f, -0.5f,0.0f, 16.0f,
                -0.5f, -0.5f, -0.5f,0.0f, 16.0f,

                -0.5f, -0.5f, -0.5f,0.0f, 16.0f,
                -0.5f, +0.5f, -0.5f,0.0f, 16.0f,
                -0.5f, +0.5f, -0.25f,0.0f, 16.0f,

                -0.5f, +0.5f, -0.25f,0.0f, 16.0f,
                -0.5f, -0.5f, -0.25f,0.0f, 16.0f,
                -0.5f, -0.5f, -0.5f,0.0f, 16.0f,//left
                
                -0.5f, +0.5f, -0.5f,0.0f, 16.0f,
                +0.5f, +0.5f, -0.5f,0.0f, 16.0f,
                +0.5f, +0.5f, -0.25f,0.0f, 16.0f,

                +0.5f, +0.5f, -0.25f,0.0f, 16.0f,
                -0.5f, +0.5f, -0.25f,0.0f, 16.0f,
                -0.5f, +0.5f, -0.5f,0.0f, 16.0f,//top
                
                +0.5f, +0.5f, -0.5f,0.0f, 16.0f,
                +0.5f, -0.5f, -0.5f,0.0f, 16.0f,
                +0.5f, -0.5f, -0.25f,0.0f, 16.0f,

                +0.5f, -0.5f, -0.25f,0.0f, 16.0f,
                +0.5f, +0.5f, -0.25f,0.0f, 16.0f,
                +0.5f, +0.5f, -0.5f,0.0f, 16.0f,//right

                +0.5f, -0.5f, -0.25f,0.0f, 16.0f,
                -0.5f, -0.5f, -0.25f,0.0f, 16.0f,
                -0.5f, +0.5f, -0.25f,0.0f, 16.0f,

                -0.5f, +0.5f, -0.25f,0.0f, 16.0f,
                +0.5f, +0.5f, -0.25f,0.0f, 16.0f,
                +0.5f, -0.5f, -0.25f,0.0f, 16.0f,//back
            }
    };

    inline static std::vector<std::vector<float>> doorModels = {

            baseDoorModel,
            rotateCoordinatesAroundYNegative90(baseDoorModel, 1),
            rotateCoordinatesAroundYNegative90(baseDoorModel, 2),
            rotateCoordinatesAroundYNegative90(baseDoorModel, 3),
            
    };

    inline static uint32_t BLOCK_ID_BITS = 0b0000'0000'0000'0000'1111'1111'1111'1111;
    inline static uint32_t BLOCK_FLAG_BITS = 0b1111'1111'1111'1111'0000'0000'0000'0000;

    inline static uint32_t BLOCK_DIRECTION_BITS = 0b0000'0000'0000'0011'0000'0000'0000'0000;
    inline static uint32_t DOOROPEN_BITS = 0b0000'0000'0000'0100'0000'0000'0000'0000;
    inline static uint32_t DOORTOP_BITS = 0b0000'0000'0000'1000'0000'0000'0000'0000;

    inline static uint32_t OPPOSITEDOOR_BITS = 0b0000'0000'0001'0000'0000'0000'0000'0000;

    inline static uint32_t getOppositeDoorBits(uint32_t input) {
        return (input & OPPOSITEDOOR_BITS) >> 20;
    };

    inline static void setOppositeDoorBits(uint32_t& input, uint32_t bit) {
        uint32_t bits = bit << 20;
        input = input & ~OPPOSITEDOOR_BITS;
        input |= bits;
    };

    inline static uint32_t getDirectionBits(uint32_t input) {
        return (input & BLOCK_DIRECTION_BITS) >> 16;
    };

    inline static void setDirectionBits(uint32_t& input, uint32_t direction) {

        uint32_t bits = direction << 16;
        input |= bits;

    };

    inline static uint32_t getDoorOpenBit(uint32_t input) {
        return (input & DOOROPEN_BITS) >> 18;
    };

    inline static void setDoorOpenBit(uint32_t& input, uint32_t open) {

        uint32_t bits = open << 18;
        input = input & ~DOOROPEN_BITS;
        input |= bits;

    };

    inline static void toggleDoorOpenBit(uint32_t& input) {
        input ^= DOOROPEN_BITS;
    };

    inline static uint32_t getDoorTopBit(uint32_t input) {
        return (input & DOORTOP_BITS) >> 19;
    };
};



#endif