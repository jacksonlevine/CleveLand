#ifndef BLOCKINFO_H
#define BLOCKINFO_H

#include "textureface.h"
#include <vector>

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
        }
    }; 

    inline static std::vector<int> transparents = {
        2, 7, 8
    };
};



#endif