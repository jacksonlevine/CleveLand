#include "glyphface.h"

void GlyphFace::setCharCode(int charCode) {
    static glm::vec2 font_atlas_topleft(288.0f / 544.0f, 0.0f);
    static float glyphWidth = 16.0f / 544;

    const int offset = charCode - 32;
    const float xOffset = (offset % 16) * glyphWidth;
    const float yOffset = (float)((int)(offset / 16)) * glyphWidth;

    this->tl = font_atlas_topleft + glm::vec2(xOffset, yOffset);
    this->tr = this->tl + glm::vec2(glyphWidth, 0.0f);
    this->br = this->tr + glm::vec2(0.0f, glyphWidth);
    this->bl = this->tl + glm::vec2(0.0f, glyphWidth);
}