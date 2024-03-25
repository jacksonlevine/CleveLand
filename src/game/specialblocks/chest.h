#ifndef CHEST_H
#define CHEST_H

#include <vector>
#include "../../util/textureface.h"
#include "../../util/vertexutils.h"

class ChestInfo {
public:

    inline static std::vector<float> getChestUVs() {
        static TextureFace straps(13,0);
        static TextureFace blank(14,0);
        static TextureFace front(15,0);

        std::vector<float> chestUVs = {
            blank.bl.x, blank.bl.y,blank.br.x, blank.br.y,
            blank.br.x, blank.br.y,blank.br.x, blank.br.y,
            blank.tr.x, blank.tr.y,blank.br.x, blank.br.y,

            blank.tr.x, blank.tr.y,blank.tl.x, blank.tl.y,
            blank.tl.x, blank.tl.y,blank.tl.x, blank.tl.y,
            blank.bl.x, blank.bl.y,blank.tl.x, blank.tl.y,

            blank.bl.x, blank.bl.y,blank.br.x, blank.br.y,
            blank.br.x, blank.br.y,blank.br.x, blank.br.y,
            blank.tr.x, blank.tr.y,blank.br.x, blank.br.y,

            blank.tr.x, blank.tr.y,blank.tl.x, blank.tl.y,
            blank.tl.x, blank.tl.y,blank.tl.x, blank.tl.y,
            blank.bl.x, blank.bl.y,blank.tl.x, blank.tl.y,

            straps.bl.x, straps.bl.y,straps.br.x, straps.br.y,
            straps.br.x, straps.br.y,straps.br.x, straps.br.y,
            straps.tr.x, straps.tr.y,straps.br.x, straps.br.y,

            straps.tr.x, straps.tr.y,straps.tl.x, straps.tl.y,
            straps.tl.x, straps.tl.y,straps.tl.x, straps.tl.y,
            straps.bl.x, straps.bl.y, straps.tl.x, straps.tl.y,

            front.bl.x, front.bl.y, front.br.x, front.br.y,
            front.br.x, front.br.y, front.br.x, front.br.y,
            front.tr.x, front.tr.y,  front.br.x, front.br.y,

            front.tr.x, front.tr.y,  front.tl.x, front.tl.y,
            front.tl.x, front.tl.y,  front.tl.x, front.tl.y,
            front.bl.x, front.bl.y,  front.tl.x, front.tl.y,

            straps.bl.x, straps.bl.y,  straps.br.x, straps.br.y,
            straps.br.x, straps.br.y,  straps.br.x, straps.br.y,
            straps.tr.x, straps.tr.y,  straps.br.x, straps.br.y,

            straps.tr.x, straps.tr.y,  straps.tr.x, straps.tr.y,
            straps.tl.x, straps.tl.y,  straps.tr.x, straps.tr.y,
            straps.bl.x, straps.bl.y,  straps.tr.x, straps.tr.y,

            blank.bl.x, blank.bl.y,  blank.br.x, blank.br.y,
            blank.br.x, blank.br.y,  blank.br.x, blank.br.y,
            blank.tr.x, blank.tr.y,  blank.br.x, blank.br.y,

            blank.tr.x, blank.tr.y,  blank.br.x, blank.br.y,
            blank.tl.x, blank.tl.y,  blank.br.x, blank.br.y,
            blank.bl.x, blank.bl.y,   blank.br.x, blank.br.y,
        };
        return chestUVs;
    }

    inline static std::vector<float> baseChestModel = {
        //player is minus z
         
            -0.5f, -0.5f, 0.5f, 0.0f, 10.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 10.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 10.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 10.0f,
            -0.5f, 0.5f, 0.5f,0.0f, 10.0f,
            -0.5f, -0.5f, 0.5f,0.0f, 10.0f,
        
            0.5f, -0.5f, -0.5f,0.0f, 10.0f,
            0.5f, -0.5f, 0.5f,0.0f, 10.0f,
            0.5f, 0.5f, 0.5f,0.0f, 10.0f,

            0.5f, 0.5f, 0.5f,0.0f, 10.0f,
            0.5f, 0.5f, -0.5f,0.0f, 10.0f,
            0.5f, -0.5f, -0.5f,0.0f, 10.0f,

            0.5f, -0.5f, 0.5f, 0.0f,14.0f,
            -0.5f, -0.5f, 0.5f,0.0f, 14.0f,
            -0.5f, 0.5f, 0.5f,0.0f, 14.0f,

            -0.5f, 0.5f, 0.5f,0.0f, 14.0f,
            0.5f, 0.5f, 0.5f,0.0f, 14.0f,
            0.5f, -0.5f, 0.5f, 0.0f,14.0f,

            -0.5f, -0.5f, -0.5f,0.0f,  14.0f,
            0.5f, -0.5f, -0.5f,0.0f, 14.0f,
            0.5f, 0.5f, -0.5f,0.0f, 14.0f,

            0.5f, 0.5f, -0.5f,0.0f, 14.0f,
            -0.5f, 0.5f, -0.5f,0.0f, 14.0f,
            -0.5f, -0.5f, -0.5f,0.0f, 14.0f,

            -0.5f, 0.5f, -0.5f,0.0f, 16.0f,
            0.5f, 0.5f, -0.5f,0.0f, 16.0f,
            0.5f, 0.5f, 0.5f,0.0f, 16.0f,

            0.5f, 0.5f, 0.5f, 0.0f,16.0f,
            -0.5f, 0.5f, 0.5f,0.0f, 16.0f,
            -0.5f, 0.5f, -0.5f,0.0f, 16.0f,

            0.5f, -0.5f, -0.5f, 0.0f,7.0f,
            -0.5f, -0.5f, -0.5f,0.0f, 7.0f,
            -0.5f, -0.5f, 0.5f,0.0f, 7.0f,

            -0.5f, -0.5f, 0.5f,0.0f, 7.0f,
            0.5f, -0.5f, 0.5f,0.0f, 7.0f,
            0.5f, -0.5f, -0.5f,0.0f,  7.0f
        
    };

    inline static std::vector<std::vector<float>> chestModels = {

            baseChestModel,
            rotateCoordinatesAroundYNegative90(baseChestModel, 1),
            rotateCoordinatesAroundYNegative90(baseChestModel, 2),
            rotateCoordinatesAroundYNegative90(baseChestModel, 3),
            
    };

};

#endif