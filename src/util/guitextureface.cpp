#include "guitextureface.h"

const float oneGuiPixel = 0.0034722222222222222222;     // 1/288      Padding
const float guiTextureWidth = 0.0555555555555555555555; // 16/288       16 pixel texture width
const float oneOver16 = 0.0625;

GUITextureFace::GUITextureFace(
    glm::vec2 tl,
    glm::vec2 bl,
    glm::vec2 br,
    glm::vec2 tr
) : tl(tl), bl(bl), br(br), tr(tr) {

}

GUITextureFace::GUITextureFace(
    int x,
    int y
) :
    bl(glm::vec2(0.0f + oneGuiPixel + (oneOver16 * (float)x), 1.0f - ((float)y * oneOver16) - oneGuiPixel)),
    tl(glm::vec2(0.0f + oneGuiPixel + (oneOver16 * (float)x), 1.0f - ((float)y * oneOver16) - guiTextureWidth - oneGuiPixel)),
    tr(glm::vec2(0.0f + oneGuiPixel + (oneOver16 * (float)x) + guiTextureWidth, 1.0f - ((float)y * oneOver16) - guiTextureWidth - oneGuiPixel)),
    br(glm::vec2(0.0f + oneGuiPixel + (oneOver16 * (float)x) + guiTextureWidth, 1.0f - ((float)y * oneOver16) - oneGuiPixel))
{

}