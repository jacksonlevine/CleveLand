#include "textureface.h"

const float onePixel = 0.0018382352941176;     // 1/544      Padding
const float textureWidth = 0.0588235294117647; // 1/17       32 pixel texture width
const float oneOver16 = 0.0625;

TextureFace::TextureFace(
    glm::vec2 tl,
    glm::vec2 bl,
    glm::vec2 br,
    glm::vec2 tr
) : tl(tl), bl(bl), br(br), tr(tr) {

}

TextureFace::TextureFace(
    int x,
    int y
) :
    br(glm::vec2(0.0f + onePixel + (oneOver16 * (float)x), 1.0f - ((float)y * oneOver16) - onePixel)),
    tr(glm::vec2(0.0f + onePixel + (oneOver16 * (float)x), 1.0f - ((float)y * oneOver16) - textureWidth - onePixel)),
    tl(glm::vec2(0.0f + onePixel + (oneOver16 * (float)x) + textureWidth, 1.0f - ((float)y * oneOver16) - textureWidth - onePixel)),
    bl(glm::vec2(0.0f + onePixel + (oneOver16 * (float)x) + textureWidth, 1.0f - ((float)y * oneOver16) - onePixel))
{

}