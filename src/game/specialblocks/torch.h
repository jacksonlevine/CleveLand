#ifndef TORCH_H
#define TORCH_H

#include <vector>
#include "../../util/textureface.h"
#include "../../util/vertexutils.h"

class TorchInfo {
public:

    inline static std::vector<float> getTorchUVs() {
        static TextureFace face(5,2);

        static TextureFace face2(5,3);
        std::vector<float> torchUVs = {

            face.bl.x, face.bl.y-(onePixel*10.0f),                  face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*4.0f), face.bl.y,                face.bl.x, face.bl.y, 

            face.bl.x+(onePixel*4.0f), face.bl.y,                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*4.0f), face.bl.y-(onePixel*10.0f), face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y-(onePixel*10.0f),                  face.bl.x, face.bl.y, 

            

            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+onePixel, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+onePixel, face.bl.y-(onePixel*10.0f),                                 face.bl.x, face.bl.y, 
            
            face.bl.x+onePixel, face.bl.y-(onePixel*10.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y-(onePixel*10.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 




            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*4.0f), face.bl.y,                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*4.0f), face.bl.y-(onePixel*10.0f), face.bl.x, face.bl.y, 

            face.bl.x+(onePixel*4.0f), face.bl.y-(onePixel*10.0f), face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y-(onePixel*10.0f),                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 




            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+onePixel, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+onePixel, face.bl.y-(onePixel*10.0f),                                 face.bl.x, face.bl.y, 
            
            face.bl.x+onePixel, face.bl.y-(onePixel*10.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y-(onePixel*10.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 



            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+onePixel, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+onePixel, face.bl.y-(onePixel*4.0f),                                 face.bl.x, face.bl.y, 
            
            face.bl.x+onePixel, face.bl.y-(onePixel*4.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y-(onePixel*4.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 



            



            face.bl.x, face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*2.0f), face.bl.y,                                 face.bl.x, face.bl.y, 

            face.bl.x+(onePixel*2.0f), face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*2.0f), face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 
            


            face.bl.x, face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*2.0f), face.bl.y,                                 face.bl.x, face.bl.y, 

            face.bl.x+(onePixel*2.0f), face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*2.0f), face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 



            face.bl.x, face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*2.0f), face.bl.y,                                 face.bl.x, face.bl.y, 

            face.bl.x+(onePixel*2.0f), face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*2.0f), face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 



            face.bl.x, face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*2.0f), face.bl.y,                                 face.bl.x, face.bl.y, 

            face.bl.x+(onePixel*2.0f), face.bl.y,                                 face.bl.x, face.bl.y, 
            face.bl.x+(onePixel*2.0f), face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 
            face.bl.x, face.bl.y-(onePixel*2.0f),                                 face.bl.x, face.bl.y, 








            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 

            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y,


            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 

            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y,



            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 

            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y,



            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 

            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y,



            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 

            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y,



            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 

            face2.bl.x+(onePixel*8.0f), face2.bl.y,                                 face2.bl.x, face2.bl.y, 
            face2.bl.x+(onePixel*8.0f), face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y, 
            face2.bl.x, face2.bl.y-(onePixel*8.0f),                                 face2.bl.x, face2.bl.y,
        };
        return torchUVs;
    }

    inline static std::vector<float> baseTorchModel = {

        //TOP PART
            -0.125f,  0.375f, 0.5f,       0.0f,   16.0f,
            -0.125f,  0.375f, -0.125f,   0.0f,   16.0f,
             0.125f,  0.375f, -0.125f, 0.0f,   16.0f,

             0.125f,  0.375f, -0.125f, 0.0f,   16.0f,
             0.125f,  0.375f, 0.5f, 0.0f,   16.0f,
             -0.125f,  0.375f, 0.5f, 0.0f,   16.0f,


            0.125f,  0.375f, -0.125f, 0.0f,   16.0f,
            0.125f,  0.375f-0.0625f, -0.125f, 0.0f,   16.0f,
            0.125f,  0.375f-0.0625f, 0.5f, 0.0f,   16.0f,

            0.125f,  0.375f-0.0625f, 0.5f, 0.0f,   16.0f,
            0.125f,  0.375f, 0.5f, 0.0f,   16.0f,
            0.125f,  0.375f, -0.125f, 0.0f,   16.0f,


            0.125f,  0.375f-0.0625f, -0.125f, 0.0f,   16.0f,
            -0.125f,  0.375f-0.0625f, -0.125f, 0.0f,   16.0f,
            -0.125f,  0.375f-0.0625f, 0.5f, 0.0f,   16.0f,

            -0.125f,  0.375f-0.0625f, 0.5f, 0.0f,   16.0f,
            0.125f,  0.375f-0.0625f, 0.5f, 0.0f,   16.0f,
            0.125f,  0.375f-0.0625f, -0.125f, 0.0f,   16.0f,


            -0.125f,  0.375f-0.0625f, -0.125f, 0.0f,   16.0f,
            -0.125f,  0.375f, -0.125f, 0.0f,   16.0f,
            -0.125f,  0.375f, 0.5f, 0.0f,   16.0f,

            -0.125f,  0.375f, 0.5f, 0.0f,   16.0f,
            -0.125f,  0.375f-0.0625f, 0.5f, 0.0f,   16.0f,
            -0.125f,  0.375f-0.0625f, -0.125f, 0.0f,   16.0f,


            -0.125f,  0.375f, -0.125f, 0.0f,   16.0f,
            -0.125f,  0.375f-0.0625f, -0.125f, 0.0f,   16.0f,
            0.125f,  0.375f-0.0625f, -0.125f, 0.0f,   16.0f,

            0.125f,  0.375f-0.0625f, -0.125f, 0.0f,   16.0f,
            0.125f,  0.375f, -0.125f, 0.0f,   16.0f,
            -0.125f,  0.375f, -0.125f, 0.0f,   16.0f,




//STEM PART



            -0.0625f, 0.375f-0.0625f,          0.125f, 0.0f,   16.0f,
            -0.0625f, 0.375f-0.0625f - 0.125f,  0.125f, 0.0f,   16.0f,
            0.0625f, 0.375f-0.0625f - 0.125f,  0.125f, 0.0f,   16.0f,

            0.0625f, 0.375f-0.0625f - 0.125f,  0.125f, 0.0f,   16.0f,
            0.0625f, 0.375f-0.0625f,  0.125f, 0.0f,   16.0f,
            -0.0625f, 0.375f-0.0625f,          0.125f, 0.0f,   16.0f,


            0.0625f, 0.375f-0.0625f,  0.125f, 0.0f,   16.0f,
            0.0625f, 0.375f-0.0625f - 0.125f,  0.125f, 0.0f,   16.0f,
            0.0625f, 0.375f-0.0625f - 0.125f,  0.125f+0.125f, 0.0f,   16.0f,

            0.0625f, 0.375f-0.0625f - 0.125f,  0.125f+0.125f, 0.0f,   16.0f,
            0.0625f, 0.375f-0.0625f,  0.125f+0.125f, 0.0f,   16.0f,
            0.0625f, 0.375f-0.0625f,  0.125f, 0.0f,   16.0f,


            0.0625f, 0.375f-0.0625f,           0.125f+0.125f, 0.0f,   16.0f,
            0.0625f, 0.375f-0.0625f - 0.125f,  0.125f+0.125f, 0.0f,   16.0f,
            -0.0625f, 0.375f-0.0625f - 0.125f,  0.125f+0.125f, 0.0f,   16.0f,

            -0.0625f, 0.375f-0.0625f - 0.125f,  0.125f+0.125f, 0.0f,   16.0f,
            -0.0625f, 0.375f-0.0625f,  0.125f+0.125f, 0.0f,   16.0f,
            0.0625f, 0.375f-0.0625f,           0.125f+0.125f, 0.0f,   16.0f,


            -0.0625f, 0.375f-0.0625f,  0.125f+0.125f, 0.0f,   16.0f,
            -0.0625f, 0.375f-0.0625f- 0.125f,  0.125f+0.125f, 0.0f,   16.0f,
            -0.0625f, 0.375f-0.0625f- 0.125f,  0.125f, 0.0f,   16.0f,

            -0.0625f, 0.375f-0.0625f- 0.125f,  0.125f, 0.0f,   16.0f,
            -0.0625f, 0.375f-0.0625f,  0.125f, 0.0f,   16.0f,
            -0.0625f, 0.375f-0.0625f,  0.125f+0.125f, 0.0f,   16.0f,





//BULB PART





            -0.25f,      0.375f-0.0625f- 0.125f,            0.125f - 0.1875f,  0.0f,   16.0f,
            -0.25f,      0.375f-0.0625f- 0.125f - 0.5f,    0.125f - 0.1875f,  0.0f,   16.0f,
            0.25f,      0.375f-0.0625f- 0.125f - 0.5f,    0.125f - 0.1875f,  0.0f,   16.0f,

            0.25f,      0.375f-0.0625f- 0.125f - 0.5f,    0.125f - 0.1875f,  0.0f,   16.0f,
            0.25f,      0.375f-0.0625f- 0.125f ,         0.125f - 0.1875f,  0.0f,   16.0f,
            -0.25f,      0.375f-0.0625f- 0.125f,            0.125f - 0.1875f,  0.0f,   16.0f,



             0.25f,      0.375f-0.0625f- 0.125f ,         0.125f - 0.1875f,  0.0f,   16.0f,
              0.25f,      0.375f-0.0625f- 0.125f-0.5f ,         0.125f - 0.1875f,  0.0f,   16.0f,
              0.25f,      0.375f-0.0625f- 0.125f-0.5f ,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,

              0.25f,      0.375f-0.0625f- 0.125f-0.5f ,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
              0.25f,      0.375f-0.0625f- 0.125f ,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
              0.25f,      0.375f-0.0625f- 0.125f ,         0.125f - 0.1875f,  0.0f,   16.0f,



              0.25f,      0.375f-0.0625f- 0.125f ,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
              0.25f,      0.375f-0.0625f- 0.125f -0.5f,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
              -0.25f,      0.375f-0.0625f- 0.125f -0.5f,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,

              -0.25f,      0.375f-0.0625f- 0.125f -0.5f,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
              -0.25f,      0.375f-0.0625f- 0.125f,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
              0.25f,      0.375f-0.0625f- 0.125f ,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,



                -0.25f,      0.375f-0.0625f- 0.125f,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
                -0.25f,      0.375f-0.0625f- 0.125f-0.5f,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
                -0.25f,      0.375f-0.0625f- 0.125f-0.5f,         0.125f - 0.1875f,  0.0f,   16.0f,

                -0.25f,      0.375f-0.0625f- 0.125f-0.5f,         0.125f - 0.1875f,  0.0f,   16.0f,
                -0.25f,      0.375f-0.0625f- 0.125f,         0.125f - 0.1875f,  0.0f,   16.0f,
                -0.25f,      0.375f-0.0625f- 0.125f,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,










                -0.25f,      0.375f-0.0625f- 0.125f,            0.125f - 0.1875f,  0.0f,   16.0f,
                0.25f,      0.375f-0.0625f- 0.125f ,         0.125f - 0.1875f, 0.0f,   16.0f,
                0.25f,      0.375f-0.0625f- 0.125f ,         0.125f + 0.125f +  0.1875f, 0.0f,   16.0f,

                0.25f,      0.375f-0.0625f- 0.125f ,         0.125f + 0.125f +  0.1875f, 0.0f,   16.0f,
                -0.25f,      0.375f-0.0625f- 0.125f,         0.125f + 0.125f +  0.1875f, 0.0f,   16.0f,
                -0.25f,      0.375f-0.0625f- 0.125f,            0.125f - 0.1875f,  0.0f,   16.0f,





                -0.25f,      0.375f-0.0625f- 0.125f - 0.5f,    0.125f - 0.1875f, 0.0f,   16.0f,
                -0.25f,      0.375f-0.0625f- 0.125f-0.5f,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
                0.25f,      0.375f-0.0625f- 0.125f-0.5f ,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,

                0.25f,      0.375f-0.0625f- 0.125f-0.5f ,         0.125f + 0.125f +  0.1875f,  0.0f,   16.0f,
                0.25f,      0.375f-0.0625f- 0.125f-0.5f ,         0.125f - 0.1875f,  0.0f,   16.0f,
                -0.25f,      0.375f-0.0625f- 0.125f - 0.5f,    0.125f - 0.1875f, 0.0f,   16.0f,
    };

    inline static std::vector<std::vector<float>> torchModels = {

            baseTorchModel,
            rotateCoordinatesAroundYNegative90(baseTorchModel, 1),
            rotateCoordinatesAroundYNegative90(baseTorchModel, 2),
            rotateCoordinatesAroundYNegative90(baseTorchModel, 3),
            
    };

};

#endif