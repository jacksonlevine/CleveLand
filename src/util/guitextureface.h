#ifndef GUITEXTUREFACE_H
#define GUITEXTUREFACE_H

#include <glm/glm.hpp>

struct GUITextureFace {
public:
    glm::vec2 tl;
    glm::vec2 bl;
    glm::vec2 br;
    glm::vec2 tr;
    
    GUITextureFace(
        glm::vec2 tl,
        glm::vec2 bl,
        glm::vec2 br,
        glm::vec2 tr
    );
    GUITextureFace(
        int x,
        int y
    );
};

extern const float oneGuiPixel;
extern const float guiTextureWidth;
extern const float oneOver16;


#endif