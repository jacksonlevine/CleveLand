#include "guielement.h"

GUIButton::GUIButton(float xOffset, float yOffset, const char *label, float manualWidth, float elementID,
    std::function<void()> function) :
    elementID(elementID),
    screenWidth(0),//TEMPORARY, FIX THESE
    screenHeight(0),//TEMPORARY, FIX THESE
    screenPos(glm::vec2(0,0)), //TEMPORARY, FIX THESE
    uploaded(false),
    vbo(0),
    xOffset(xOffset),
    yOffset(yOffset),
    label(label),
    manualWidth(manualWidth),
    myFunction(function)
{
    rebuildDisplayData();
}

void GUIButton::rebuildDisplayData() {

    float xOffset = this->xOffset*700 / windowWidth;
    float yOffset = this->yOffset*700 / windowHeight;

    displayData.clear();
    glDeleteBuffers(1, &vbo);
    glGenBuffers(1, &vbo);

    TextureFace leftEnd(0,0);
    TextureFace rightEnd(1,0);

    TextureFace middle(0,0);
    middle.bl.x += textureWidth/2;
    middle.tl.x += textureWidth/2;
    
    float lettersCount = std::strlen(label);
    float unitWidth = (32.0f/windowWidth)*2;
    float unitHeight = (32.0f/windowHeight)*2;

    float letHeight = (32.0f/windowHeight);
    float letWidth = (32.0f/windowWidth);

    float totwid = unitWidth * 2 + letWidth * lettersCount;
    float tothei = unitHeight;
    float totletwid = letWidth * lettersCount;


    float manWid = (manualWidth * 1280.0f) / windowWidth;

    totwid = std::max(manWid, totwid);

    glm::vec2 leftStart(-totwid/2 + xOffset, -tothei/2 + yOffset);
    glm::vec2 middleStart(leftStart.x + unitWidth, leftStart.y);
    glm::vec2 rightStart(totwid/2 - unitWidth + xOffset, leftStart.y);

    glm::vec2 letterStart(-totletwid/2 + xOffset, -letHeight/2 + yOffset);

    screenWidth = totwid/2;
    screenHeight = tothei/2;
    screenPos = glm::vec2(
        0.5 + (leftStart.x/2),
        0.5 - (tothei/2 + yOffset)/2
    );

    GlyphFace glyph;

    for(int i = 0; i < lettersCount; i++) {
        glyph.setCharCode(static_cast<int>(label[i]));
        glm::vec2 thisLetterStart(letterStart.x + i*letWidth, letterStart.y);
        displayData.insert(displayData.end(), {
            thisLetterStart.x, thisLetterStart.y,                     glyph.bl.x, glyph.bl.y, -1.0f,
            thisLetterStart.x, thisLetterStart.y+letHeight,           glyph.tl.x, glyph.tl.y, -1.0f,
            thisLetterStart.x+letWidth, thisLetterStart.y+letHeight, glyph.tr.x, glyph.tr.y, -1.0f,

            thisLetterStart.x+letWidth, thisLetterStart.y+letHeight, glyph.tr.x, glyph.tr.y, -1.0f,
            thisLetterStart.x+letWidth, thisLetterStart.y,           glyph.br.x, glyph.br.y, -1.0f,
            thisLetterStart.x, thisLetterStart.y,                     glyph.bl.x, glyph.bl.y, -1.0f
        });
    }

      //X, Y, U, V, ElementID
    displayData.insert(displayData.end(), {
        leftStart.x, leftStart.y,                       leftEnd.bl.x, leftEnd.bl.y,   elementID,
        leftStart.x, leftStart.y+unitHeight,             leftEnd.tl.x, leftEnd.tl.y,   elementID,
        leftStart.x+unitWidth, leftStart.y+unitHeight,   leftEnd.tr.x, leftEnd.tr.y,   elementID,

        leftStart.x+unitWidth, leftStart.y+unitHeight,   leftEnd.tr.x, leftEnd.tr.y,   elementID,
        leftStart.x+unitWidth, leftStart.y,             leftEnd.br.x, leftEnd.br.y,   elementID,
        leftStart.x, leftStart.y,                       leftEnd.bl.x, leftEnd.bl.y,   elementID,

        middleStart.x, middleStart.y,                   middle.bl.x, middle.bl.y,     elementID,
        middleStart.x, middleStart.y+unitHeight,         middle.tl.x, middle.tl.y,     elementID,
        rightStart.x, rightStart.y+unitHeight,           middle.tr.x, middle.tr.y,     elementID,

        rightStart.x, rightStart.y+unitHeight,           middle.tr.x, middle.tr.y,     elementID,
        rightStart.x, rightStart.y,                     middle.br.x, middle.br.y,     elementID,
        middleStart.x, middleStart.y,                   middle.bl.x, middle.bl.y,     elementID,

        rightStart.x, rightStart.y,                     rightEnd.bl.x, rightEnd.bl.y, elementID,
        rightStart.x, rightStart.y+unitHeight,           rightEnd.tl.x, rightEnd.tl.y, elementID,
        rightStart.x+unitWidth, rightStart.y+unitHeight, rightEnd.tr.x, rightEnd.tr.y, elementID,
        
        rightStart.x+unitWidth, rightStart.y+unitHeight, rightEnd.tr.x, rightEnd.tr.y, elementID,
        rightStart.x+unitWidth, rightStart.y,           rightEnd.br.x, rightEnd.br.y, elementID,
        rightStart.x, rightStart.y,                     rightEnd.bl.x, rightEnd.bl.y, elementID
    }); 
}