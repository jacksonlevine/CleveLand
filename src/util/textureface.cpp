#include "textureface.h"

const float onePixel = 0.00183823529411764705882352941176f;     //  1/544      Padding
const float textureWidth = 0.02941176470588235294117647058824f; // 16/544      16 pixel texture width
const float texSlotWidth = 0.03308823529411764705882352941176f; // 18/544      Texture slot width, padding included

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
    bl(glm::vec2(0.0f + onePixel + (texSlotWidth * (float)x), 1.0f - ((float)y * texSlotWidth) - onePixel)),
    tl(glm::vec2(0.0f + onePixel + (texSlotWidth * (float)x), 1.0f - ((float)y * texSlotWidth) - textureWidth - onePixel)),
    tr(glm::vec2(0.0f + onePixel + (texSlotWidth * (float)x) + textureWidth, 1.0f - ((float)y * texSlotWidth) - textureWidth - onePixel)),
    br(glm::vec2(0.0f + onePixel + (texSlotWidth * (float)x) + textureWidth, 1.0f - ((float)y * texSlotWidth) - onePixel))
{

}