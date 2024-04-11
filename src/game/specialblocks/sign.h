#ifndef SIGN_H
#define SIGN_H

#include <vector>
#include "../../util/textureface.h"
#include "../../util/vertexutils.h"
#include "../../util/chunkcoord.h"
#include "../signwords.h"
#include "../../gui/glyphface.h"

class SignInfo {
public:

    inline static std::vector<float> getSignBaseUVs() {
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




    //0 is verts and 1 is uvs
    inline static std::vector<std::vector<float>> getSignModel(BlockCoord where, int direction, bool large = false) {
        std::vector<std::vector<float>> model;

        std::vector<float> verts;
        std::vector<float> uvs;


        std::vector<float> baseUVs = getSignBaseUVs();


        verts.insert(verts.end(), baseSignModel.begin(), baseSignModel.end());
        uvs.insert(uvs.end(), baseUVs.begin(), baseUVs.end());


        glm::vec3 coord1(baseSignModel[0],baseSignModel[1],baseSignModel[2] );
        glm::vec3 coord2(baseSignModel[5],baseSignModel[6],baseSignModel[7] );

        glm::vec3 dir = coord2 - coord1;  //The right hand direction of the sign in this orientation

        auto signWordIt = signWords.find(where);

        int lettersPerRow = large ? 1 : 14;



        float glyphWidth = 1.0f / static_cast<float>(lettersPerRow);
        


        glm::vec3 s(0.5f, 0.5f, 0.36);

        if(signWordIt != signWords.end()) {
            std::string word = signWordIt->second;

            GlyphFace g;

            int length;
            if(large) {
                length = 1;
            } else {
                length = word.length();
            }
            for(int i = 0; i < length; i++) {
                char letter = word[i];
                float xpos = s.x - ((i % lettersPerRow) * glyphWidth);
                float ypos = s.y - (std::floor(static_cast<float>(i) / static_cast<float>(lettersPerRow)))*glyphWidth;

                g.setCharCode(static_cast<int>(letter));

                verts.insert(verts.end(), {
                    xpos, ypos,           s.z, 0.0f, 16.0f,
                    xpos-glyphWidth, ypos, s.z, 0.0f, 16.0f,
                    xpos-glyphWidth, ypos-glyphWidth, s.z, 0.0f, 16.0f,

                    xpos-glyphWidth, ypos-glyphWidth, s.z, 0.0f, 16.0f,
                    xpos, ypos-glyphWidth, s.z, 0.0f, 16.0f,
                    xpos, ypos,           s.z, 0.0f, 16.0f,
                });

                uvs.insert(uvs.end(), {

                    g.tl.x, g.tl.y,     g.tl.x, g.tl.y,
                     g.tr.x, g.tr.y,     g.tl.x, g.tl.y, 
                     g.br.x, g.br.y,     g.tl.x, g.tl.y,

                     g.br.x, g.br.y,     g.tl.x, g.tl.y, 
                     g.bl.x, g.bl.y,     g.tl.x, g.tl.y, 
                     g.tl.x, g.tl.y,     g.tl.x, g.tl.y, 
                });
            }


        }


        if(direction > 0) {
            verts = rotateCoordinatesAroundYNegative90(verts, direction);
        }
        

        model.push_back(verts);
        model.push_back(uvs);

        return model;

    }

};

#endif