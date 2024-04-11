#ifndef SIGN_H
#define SIGN_H

#include <vector>
#include "../../util/textureface.h"
#include "../../util/vertexutils.h"
#include "../../util/chunkcoord.h"
#include "../signwords.h"

class SignInfo {
public:

    inline static std::vector<float> getSignUVs() {
        static TextureFace face(10,1);

        std::vector<float> signUVs = {
            
            face.bl.x, face.bl.y, face.bl.x, face.bl.y,
            face.br.x, face.br.y,face.bl.x, face.bl.y,
            face.tr.x, face.tr.y,face.bl.x, face.bl.y,

            face.tr.x, face.tr.y,face.bl.x, face.bl.y,
            face.tl.x, face.tl.y,face.bl.x, face.bl.y,
            face.bl.x, face.bl.y,face.bl.x, face.bl.y,

            face.bl.x, face.bl.y,face.bl.x, face.bl.y,
            face.br.x, face.br.y,face.bl.x, face.bl.y,
            face.tr.x, face.tr.y,face.bl.x, face.bl.y,

            face.tr.x, face.tr.y,face.bl.x, face.bl.y,
            face.tl.x, face.tl.y,face.bl.x, face.bl.y,
            face.bl.x, face.bl.y,face.bl.x, face.bl.y,

        };
        return signUVs;
    }



    inline static std::vector<float> baseSignModel = {
            -0.5f, -0.5f, 0.4f,0.0f,  14.0f,
            0.5f, -0.5f, 0.4f,0.0f, 14.0f,
            0.5f, 0.5f, 0.4f,0.0f, 14.0f,

            0.5f, 0.5f, 0.4f,0.0f, 14.0f,
            -0.5f, 0.5f, 0.4f,0.0f, 14.0f,
            -0.5f, -0.5f, 0.4f,0.0f, 14.0f,



            0.5f, -0.5f, 0.5f, 0.0f,14.0f,
            -0.5f, -0.5f, 0.5f,0.0f, 14.0f,
            -0.5f, 0.5f, 0.5f,0.0f, 14.0f,

            -0.5f, 0.5f, 0.5f,0.0f, 14.0f,
            0.5f, 0.5f, 0.5f,0.0f, 14.0f,
            0.5f, -0.5f, 0.5f, 0.0f,14.0f,

            
    };

    inline static std::vector<std::vector<float>> signModels = {

            baseSignModel,
            rotateCoordinatesAroundYNegative90(baseSignModel, 1),
            rotateCoordinatesAroundYNegative90(baseSignModel, 2),
            rotateCoordinatesAroundYNegative90(baseSignModel, 3),
            
    };





    inline static std::vector<float> getSignModel(BlockCoord where, int direction) {
        std::vector<float>& thisBase = signModels[direction];

        glm::vec3 coord1(thisBase[0],thisBase[1],thisBase[2] );
        glm::vec3 coord2(thisBase[5],thisBase[6],thisBase[7] );

        glm::vec3 dir = coord2 - coord1;  //The right hand direction of the sign in this orientation

        auto signWordIt = signWords.find(where);

        int lettersPerRow = 14;



        float glyphWidth = 1.0f / static_cast<float>(lettersPerRow);
        


        glm::vec3 s(-0.5f, 0.5f, 0.36);

        if(signWordIt != signWords.end()) {
            std::string word = signWordIt->second;


            for(int i = 0; i < word.length(); i++) {
                char letter = word[i];
                float xpos = s.x + ((i % lettersPerRow) * glyphWidth);
                
            }


        }

        



    }

};

#endif