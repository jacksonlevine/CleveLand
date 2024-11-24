#ifndef STAIR_H
#define STAIR_H

#include <vector>
#include "../../util/textureface.h"
#include "../../util/vertexutils.h"

class StairInfo {
public:

    inline static std::vector<float> getStairUVs() {
        static TextureFace face(9, 0);

        std::vector<float> stairUVs = {
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,

            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,


            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x + textureWidth/2.0f, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x + textureWidth/2.0f, face.bl.y - textureWidth, face.bl.x, face.bl.y,

            face.bl.x + textureWidth/2.0f, face.bl.y - textureWidth, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,

            face.bl.x, face.bl.y, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.br.x, face.br.y, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,

            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth, face.bl.x, face.bl.y,
            face.bl.x + textureWidth/2.0f, face.bl.y - textureWidth, face.bl.x, face.bl.y,
            face.bl.x + textureWidth/2.0f, face.bl.y - textureWidth, face.bl.x, face.bl.y,
            face.bl.x + textureWidth/2.0f, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,


            face.bl.x, face.bl.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y, face.bl.x, face.bl.y,
            face.tr.x, face.tr.y, face.bl.x, face.bl.y,

            face.tr.x, face.tr.y, face.bl.x, face.bl.y,
            face.tl.x, face.tl.y, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,


            face.bl.x, face.bl.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y, face.bl.x, face.bl.y,
            face.tr.x, face.tr.y, face.bl.x, face.bl.y,

            face.tr.x, face.tr.y, face.bl.x, face.bl.y,
            face.tl.x, face.tl.y, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,

            face.bl.x, face.bl.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,

            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,

            face.bl.x, face.bl.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,

            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,

            face.bl.x, face.bl.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,

            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,

            face.bl.x, face.bl.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,

            face.br.x, face.br.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y - textureWidth/2.0f, face.bl.x, face.bl.y,
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,



        };
        return stairUVs;
    }

    inline static std::vector<float> baseStairModel = {
        //player is minus z
         ///left bottom half
            -0.5f, -0.5f, 0.5f, 0.0f, 10.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 10.0f,
            -0.5f, 0.0f, -0.5f, 0.0f, 10.0f,

        -0.5f, 0.0f, -0.5f, 0.0f, 10.0f,
        -0.5f, 0.0f, 0.5f, 0.0f, 10.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 10.0f,


        //left top half
        -0.5f, 0.0f, 0.5f, 0.0f, 10.0f,
        -0.5f, 0.0f, 0.0f, 0.0f, 10.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 10.0f,

        -0.5f, 0.5f, 0.0f, 0.0f, 10.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 10.0f,
        -0.5f, 0.0f, 0.5f, 0.0f, 10.0f,

        //right bottom half
        0.5f, -0.5f, 0.5f, 0.0f, 10.0f,
        0.5f, 0.0f, 0.5f, 0.0f, 10.0f,
        0.5f, 0.0f, -0.5f, 0.0f, 10.0f,

        0.5f, 0.0f, -0.5f, 0.0f, 10.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 10.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 10.0f,

        //right top half
        0.5f, 0.0f, 0.5f, 0.0f, 10.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 10.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 10.0f,

        0.5f, 0.5f, 0.0f, 0.0f, 10.0f,
        0.5f, 0.0f, 0.0f, 0.0f, 10.0f,
        0.5f, 0.0f, 0.5f, 0.0f, 10.0f,


        //bottom
        -0.5f, -0.5f, 0.5f, 0.0f, 8.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 8.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 8.0f,

        0.5f, -0.5f, -0.5f, 0.0f, 8.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 8.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 8.0f,

        //back
        0.5f, -0.5f, 0.5f, 0.0f, 8.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 8.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 8.0f,

        -0.5f, 0.5f, 0.5f, 0.0f, 8.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 8.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 8.0f,



        //front
        -0.5f, -0.5f, -0.5f, 0.0f, 10.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 10.0f,
        0.5f, 0.0f, -0.5f, 0.0f, 10.0f,

        0.5f, 0.0f, -0.5f, 0.0f, 10.0f,
        -0.5f, 0.0f, -0.5f, 0.0f, 10.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 10.0f,

        //front top
        -0.5f, 0.0f, -0.5f, 0.0f, 10.0f,
        0.5f, 0.0f, -0.5f, 0.0f, 10.0f,
        0.5f, 0.0f, 0.0f, 0.0f, 10.0f,

        0.5f, 0.0f, 0.0f, 0.0f, 10.0f,
        -0.5f, 0.0f, 0.0f, 0.0f, 10.0f,
        -0.5f, 0.0f, -0.5f, 0.0f, 10.0f,

        //top front
        -0.5f, 0.0f, 0.0f, 0.0f, 10.0f,
        0.5f, 0.0f, 0.0f, 0.0f, 10.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 10.0f,

        0.5f, 0.5f, 0.0f, 0.0f, 10.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 10.0f,
        -0.5f, 0.0f, 0.0f, 0.0f, 10.0f,


        //top top
        -0.5f, 0.5f, 0.0f, 0.0f, 10.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 10.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 10.0f,

        0.5f, 0.5f, 0.5f, 0.0f, 10.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 10.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 10.0f,

    };

    inline static std::vector<std::vector<float>> stairModels = {

            baseStairModel,
            rotateCoordinatesAroundYNegative90(baseStairModel, 1),
            rotateCoordinatesAroundYNegative90(baseStairModel, 2),
            rotateCoordinatesAroundYNegative90(baseStairModel, 3),
            
    };

};

#endif