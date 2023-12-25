#ifndef GLYPHFACE_H
#define GLYPHFACE_H
#include <glm/glm.hpp>
class GlyphFace
{
public:
    glm::vec2 tl;
    glm::vec2 tr;
    glm::vec2 bl;
    glm::vec2 br;
    void setCharCode(int charCode);
};
#endif