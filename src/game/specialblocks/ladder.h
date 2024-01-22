#ifndef LADDER_H
#define LADDER_H

#include <vector>
#include "../../util/textureface.h"
#include "../../util/vertexutils.h"

class LadderInfo {
public:

    inline static std::vector<float> getLadderUVs() {
        static TextureFace face(0,1);

        std::vector<float> ladderUVs = {
            
            face.bl.x, face.bl.y,
            face.br.x, face.br.y,
            face.tr.x, face.tr.y,

            face.tr.x, face.tr.y,
            face.tl.x, face.tl.y,
            face.bl.x, face.bl.y,

            face.bl.x, face.bl.y,
            face.br.x, face.br.y,
            face.tr.x, face.tr.y,

            face.tr.x, face.tr.y,
            face.tl.x, face.tl.y,
            face.bl.x, face.bl.y,

        };
        return ladderUVs;
    }

    inline static std::vector<float> baseLadderModel = {
            0.5f, -0.5f, 0.5f, 0.0f,14.0f,
            -0.5f, -0.5f, 0.5f,0.0f, 14.0f,
            -0.5f, 0.5f, 0.5f,0.0f, 14.0f,

            -0.5f, 0.5f, 0.5f,0.0f, 14.0f,
            0.5f, 0.5f, 0.5f,0.0f, 14.0f,
            0.5f, -0.5f, 0.5f, 0.0f,14.0f,

            -0.5f, -0.5f, 0.4f,0.0f,  14.0f,
            0.5f, -0.5f, 0.4f,0.0f, 14.0f,
            0.5f, 0.5f, 0.4f,0.0f, 14.0f,

            0.5f, 0.5f, 0.4f,0.0f, 14.0f,
            -0.5f, 0.5f, 0.4f,0.0f, 14.0f,
            -0.5f, -0.5f, 0.4f,0.0f, 14.0f
    };

    inline static std::vector<std::vector<float>> ladderModels = {

            baseLadderModel,
            rotateCoordinatesAroundYNegative90(baseLadderModel, 1),
            rotateCoordinatesAroundYNegative90(baseLadderModel, 2),
            rotateCoordinatesAroundYNegative90(baseLadderModel, 3),
            
    };

};

#endif