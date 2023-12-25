#include "guielement.h"

std::vector<float> createButton(float xOffset, float yOffset, const char *label, float manualWidth, float elementID) {
    std::vector<float> data;

    TextureFace leftEnd(0,0);
    TextureFace rightEnd(1,0);

    TextureFace middle(0,0);
    middle.bl.x += textureWidth/2;
    middle.tl.x += textureWidth/2;
    
    float lettersCount = std::strlen(label);
    float unitWidth = 32.0f/1280;

    float totwid = unitWidth * (2 + lettersCount);
    float tothei = unitWidth;
    float totletwid = unitWidth * lettersCount;

    totwid = std::max(manualWidth, totwid);

    glm::vec2 leftStart(-totwid/2 + xOffset, -tothei/2 + yOffset);
    glm::vec2 middleStart(leftStart.x + unitWidth, leftStart.y);
    glm::vec2 rightStart(totwid/2 - unitWidth + xOffset, leftStart.y);

    glm::vec2 letterStart(-totletwid/2 + xOffset, leftStart.y);

    //X, Y, U, V, ElementID
    data.insert(data.end(), {
        leftStart.x, leftStart.y,                       leftEnd.bl.x, leftEnd.bl.y,   elementID,
        leftStart.x, leftStart.y+unitWidth,             leftEnd.tl.x, leftEnd.tl.y,   elementID,
        leftStart.x+unitWidth, leftStart.y+unitWidth,   leftEnd.tr.x, leftEnd.tr.y,   elementID,

        leftStart.x+unitWidth, leftStart.y+unitWidth,   leftEnd.tr.x, leftEnd.tr.y,   elementID,
        leftStart.x+unitWidth, leftStart.y,             leftEnd.br.x, leftEnd.br.y,   elementID,
        leftStart.x, leftStart.y,                       leftEnd.bl.x, leftEnd.bl.y,   elementID,

        middleStart.x, middleStart.y,                   middle.bl.x, middle.bl.y,     elementID,
        middleStart.x, middleStart.y+unitWidth,         middle.tl.x, middle.tl.y,     elementID,
        rightStart.x, rightStart.y+unitWidth,           middle.tr.x, middle.tr.y,     elementID,

        rightStart.x, rightStart.y+unitWidth,           middle.tr.x, middle.tr.y,     elementID,
        rightStart.x, rightStart.y,                     middle.br.x, middle.br.y,     elementID,
        middleStart.x, middleStart.y,                   middle.bl.x, middle.bl.y,     elementID,

        rightStart.x, rightStart.y,                     rightEnd.bl.x, rightEnd.bl.y, elementID,
        rightStart.x, rightStart.y+unitWidth,           rightEnd.tl.x, rightEnd.tl.y, elementID,
        rightStart.x+unitWidth, rightStart.y+unitWidth, rightEnd.tr.x, rightEnd.tr.y, elementID,
        
        rightStart.x+unitWidth, rightStart.y+unitWidth, rightEnd.tr.x, rightEnd.tr.y, elementID,
        rightStart.x+unitWidth, rightStart.y,           rightEnd.br.x, rightEnd.br.y, elementID,
        rightStart.x, rightStart.y,                     rightEnd.bl.x, rightEnd.bl.y, elementID
    }); 

    GlyphFace glyph;

    for(int i = 0; i < lettersCount; i++) {
        glyph.setCharCode(static_cast<int>(label[i]));
        glm::vec2 thisLetterStart(letterStart.x + i*unitWidth, letterStart.y);
        data.insert(data.end(), {
            thisLetterStart.x, thisLetterStart.y,                     glyph.bl.x, glyph.bl.y, -1.0f,
            thisLetterStart.x, thisLetterStart.y+unitWidth,           glyph.tl.x, glyph.tl.y, -1.0f,
            thisLetterStart.x+unitWidth, thisLetterStart.y+unitWidth, glyph.tr.x, glyph.tr.y, -1.0f,

            thisLetterStart.x+unitWidth, thisLetterStart.y+unitWidth, glyph.tr.x, glyph.tr.y, -1.0f,
            thisLetterStart.x+unitWidth, thisLetterStart.y,           glyph.br.x, glyph.br.y, -1.0f,
            thisLetterStart.x, thisLetterStart.y,                     glyph.bl.x, glyph.bl.y, -1.0f
        });
    }

    return data;
}