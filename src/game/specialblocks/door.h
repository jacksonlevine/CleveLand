#ifndef DOOR_H
#define DOOR_H

#include <vector>
#include "../../util/textureface.h"
#include "../../util/vertexutils.h"

class DoorInfo {
public:
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
         //player is minus z
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
            
    };

    inline static std::vector<std::vector<float>> doorModels = {

            baseDoorModel,
            rotateCoordinatesAroundYNegative90(baseDoorModel, 1),
            rotateCoordinatesAroundYNegative90(baseDoorModel, 2),
            rotateCoordinatesAroundYNegative90(baseDoorModel, 3),
            
    };

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