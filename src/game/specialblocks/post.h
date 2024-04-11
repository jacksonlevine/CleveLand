#ifndef POST_H
#define POST_H

#include <vector>
#include "../../util/textureface.h"
#include "../../util/vertexutils.h"

class PostInfo {
public:
    inline static float width = 0.21875f;
    inline static std::vector<float> getPostUVs() {
        static TextureFace face(5,1);
        static TextureFace face2(5,4);


        std::vector<float> postUVs = {
            
            face.bl.x, face.bl.y,                   face.bl.x, face.bl.y,
            face.bl.x + (textureWidth*(width*2.0f)), face.bl.y,    face.bl.x, face.bl.y,
            face.bl.x + (textureWidth*(width*2.0f)), face.tr.y,     face.bl.x, face.bl.y,

            face.bl.x + (textureWidth*(width*2.0f)), face.tr.y,     face.bl.x, face.bl.y,
            face.bl.x, face.tr.y,                    face.bl.x, face.bl.y,
            face.bl.x, face.bl.y,                   face.bl.x, face.bl.y,
            

            face.bl.x, face.bl.y,                   face.bl.x, face.bl.y,
            face.bl.x + (textureWidth*(width*2.0f)), face.bl.y,    face.bl.x, face.bl.y,
            face.bl.x + (textureWidth*(width*2.0f)), face.tr.y,     face.bl.x, face.bl.y,

            face.bl.x + (textureWidth*(width*2.0f)), face.tr.y,     face.bl.x, face.bl.y,
            face.bl.x, face.tr.y,                    face.bl.x, face.bl.y,
            face.bl.x, face.bl.y,                   face.bl.x, face.bl.y,

            face.bl.x, face.bl.y,                   face.bl.x, face.bl.y,
            face.bl.x + (textureWidth*(width*2.0f)), face.bl.y,    face.bl.x, face.bl.y,
            face.bl.x + (textureWidth*(width*2.0f)), face.tr.y,     face.bl.x, face.bl.y,

            face.bl.x + (textureWidth*(width*2.0f)), face.tr.y,     face.bl.x, face.bl.y,
            face.bl.x, face.tr.y,                    face.bl.x, face.bl.y,
            face.bl.x, face.bl.y,                   face.bl.x, face.bl.y,

            face.bl.x, face.bl.y,                   face.bl.x, face.bl.y,
            face.bl.x + (textureWidth*(width*2.0f)), face.bl.y,    face.bl.x, face.bl.y,
            face.bl.x + (textureWidth*(width*2.0f)), face.tr.y,     face.bl.x, face.bl.y,

            face.bl.x + (textureWidth*(width*2.0f)), face.tr.y,     face.bl.x, face.bl.y,
            face.bl.x, face.tr.y,                    face.bl.x, face.bl.y,
            face.bl.x, face.bl.y,                   face.bl.x, face.bl.y,




            face2.bl.x, face2.bl.y,     face2.bl.x, face2.bl.y,
            face2.bl.x+ (textureWidth*(width*2.0f)), face2.bl.y,     face2.bl.x, face2.bl.y,
            face2.bl.x+ (textureWidth*(width*2.0f)), face2.bl.y-(textureWidth*(width*2.0f)),     face2.bl.x, face2.bl.y,

            face2.bl.x+ (textureWidth*(width*2.0f)), face2.bl.y-(textureWidth*(width*2.0f)),     face2.bl.x, face2.bl.y,
            face2.bl.x, face2.bl.y-(textureWidth*(width*2.0f)),     face2.bl.x, face2.bl.y,
            face2.bl.x, face2.bl.y,     face2.bl.x, face2.bl.y,



            face2.bl.x, face2.bl.y,     face2.bl.x, face2.bl.y,
            face2.bl.x+ (textureWidth*(width*2.0f)), face2.bl.y,     face2.bl.x, face2.bl.y,
            face2.bl.x+ (textureWidth*(width*2.0f)), face2.bl.y-(textureWidth*(width*2.0f)),     face2.bl.x, face2.bl.y,

            face2.bl.x+ (textureWidth*(width*2.0f)), face2.bl.y-(textureWidth*(width*2.0f)),     face2.bl.x, face2.bl.y,
            face2.bl.x, face2.bl.y-(textureWidth*(width*2.0f)),     face2.bl.x, face2.bl.y,
            face2.bl.x, face2.bl.y,     face2.bl.x, face2.bl.y,
        };
        return postUVs;
    }


    inline static std::vector<float> basePostModel = {
            -width, -0.5f, -width, 0.0f,  14.0f,
            width, -0.5f, -width, 0.0f,  14.0f,
            width, 0.5f, -width, 0.0f,  14.0f,

            width, 0.5f, -width,0.0f,  14.0f,
            -width, 0.5f, -width,0.0f,  14.0f,
            -width, -0.5f, -width,0.0f,  14.0f,



            width, -0.5f, -width, 0.0f,  12.0f,
            width, -0.5f, width, 0.0f,  12.0f,
            width, 0.5f, width, 0.0f,  12.0f,

            width, 0.5f, width, 0.0f,  12.0f,
            width, 0.5f, -width, 0.0f,  12.0f,
            width, -0.5f, -width, 0.0f,  12.0f,



            width, -0.5f, width, 0.0f,  14.0f,
            -width, -0.5f, width, 0.0f,  14.0f,
            -width, 0.5f, width, 0.0f,  14.0f,

            -width, 0.5f, width, 0.0f,  14.0f,
            width, 0.5f, width, 0.0f,  14.0f,
            width, -0.5f, width, 0.0f,  14.0f,




            -width, -0.5f, width, 0.0f,  15.0f,
            -width, -0.5f, -width, 0.0f,  15.0f,
            -width, 0.5f, -width, 0.0f,  15.0f,

            -width, 0.5f, -width, 0.0f,  15.0f,
            -width, 0.5f, width, 0.0f,  15.0f,
            -width, -0.5f, width, 0.0f,  15.0f,











            -width, 0.5f, -width, 0.0f,  14.0f,
            width, 0.5f, -width, 0.0f,  14.0f,
            width, 0.5f, width, 0.0f,  14.0f,

            width, 0.5f, width, 0.0f,  14.0f,
            -width, 0.5f, width, 0.0f,  14.0f,
            -width, 0.5f, -width, 0.0f,  14.0f,




            -width, -0.5f, width, 0.0f,  15.0f,
            width, -0.5f, width, 0.0f,  15.0f,
            width, -0.5f, -width, 0.0f,  15.0f,

            width, -0.5f, -width, 0.0f,  15.0f,
            -width, -0.5f, -width, 0.0f,  15.0f,
            -width, -0.5f, width, 0.0f,  15.0f,

    };

    inline static std::vector<std::vector<float>> postModels = {

            basePostModel,
            basePostModel,
            basePostModel,
            basePostModel
    };

};

#endif