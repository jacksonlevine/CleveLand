#include "guielement.h"
#include <iostream>

int GUILwindowWidth = 1280;
int GUILwindowHeight = 720;

std::mutex GUIMutex;

void rebuildGUILDisplayData(GUIElement* guil) {
    // Call the specific implementation for GUIButton
    if (auto* b = dynamic_cast<GUIButton*>(guil)) {
        b->rebuildDisplayData();
        std::cout << "Rebuild button\n"; 
    }
    // Call the specific implementation for GUITextInput
    else if (auto* t = dynamic_cast<GUITextInput*>(guil)) {
        t->rebuildDisplayData();
        std::cout << "Rebuilt GUITextIn\n";
    } else {
        std::cout << "Rebuilt nothing\n";
    }
}

void updateGUIL(GUIElement* guil, KeyInput in) {
    // Call the specific implementation for GUIButton
    if (auto* b = dynamic_cast<GUIButton*>(guil)) {
        b->update(in);
    }
    // Call the specific implementation for GUITextInput
    else if (auto* t = dynamic_cast<GUITextInput*>(guil)) {
        t->update(in);
    }
}

GUIButton::GUIButton(float xOffset, float yOffset, const char *label, float manualWidth, float elementID,
    std::function<void()> function) :
    GUIElement(),

    xOffset(xOffset),
    yOffset(yOffset),
    manualWidth(manualWidth)

{
    this->textThingFlickering = false;
    this->screenWidth = 0;//TEMPORARY, FIX THESE
    this->screenHeight = 0;//TEMPORARY, FIX THESE
    this->screenPos=glm::vec2(0,0); //TEMPORARY, FIX THESE
    this->uploaded=false;
    this->vbo=0;
    this->label = label;
    this->elementID = elementID;
    this->myFunction = function;
    rebuildDisplayData();
}

void GUIButton::update(KeyInput in) {
    rebuildDisplayData();
}

GUITextInput::GUITextInput(float xOffset, float yOffset, const char *label, float manualWidth, float elementID,
    std::function<void()> function, std::string& storage) :
    GUIElement(),

    xOffset(xOffset),
    yOffset(yOffset),
    manualWidth(manualWidth),
    storage(storage)
{
    this->textThingFlickering = false;
    this->screenWidth = 0;//TEMPORARY, FIX THESE
    this->screenHeight = 0;//TEMPORARY, FIX THESE
    this->screenPos=glm::vec2(0,0); //TEMPORARY, FIX THESE
    this->uploaded=false;
    this->vbo=0;
    this->label = std::string(storage);
    this->elementID = elementID;
    this->myFunction = function;

    
    rebuildDisplayData();
}


void GUIButton::rebuildDisplayData() {

    float xOffset = this->xOffset*700 / GUILwindowWidth;
    float yOffset = this->yOffset*700 / GUILwindowHeight;

    displayData.clear();
    glDeleteBuffers(1, &vbo);
    glGenBuffers(1, &vbo);

    GUITextureFace leftEnd(0,0);
    GUITextureFace rightEnd(1,0);

    GUITextureFace middle(0,0);
    middle.bl.x += guiTextureWidth/2;
    middle.tl.x += guiTextureWidth/2;
    
    float lettersCount = std::strlen(label.c_str());
    float unitWidth = (32.0f/GUILwindowWidth)*2;
    float unitHeight = (32.0f/GUILwindowHeight)*2;

    float letHeight = (32.0f/GUILwindowHeight);
    float letWidth = (32.0f/GUILwindowWidth);

    float totwid = unitWidth * 2 + letWidth * lettersCount;
    float tothei = unitHeight;
    float totletwid = letWidth * lettersCount;


    float manWid = (manualWidth * 1280.0f) / GUILwindowWidth;

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

void GUITextInput::update(KeyInput in) {
    
    static float flickerTimer = 0.0f;
    static float flickerInterval = 0.5f;

    static float lastFrame = glfwGetTime();
    static float deltTime = 0.0f;

    if(!in.empty) {

        if(in.backspace && label.size() > 0) {
            label.pop_back();
        } else if(!in.backspace && label.size() < 128){
            label += std::string(1, in.character);
        }

        this->storage.clear();
        this->storage.insert(this->storage.begin(), label.begin(), label.end());
        
        rebuildDisplayData();
        this->uploaded = false;
    }

    float currentFrame = glfwGetTime();
    deltTime = currentFrame - lastFrame;
    lastFrame = currentFrame;


    if(flickerTimer > flickerInterval) {
        textThingFlickering = !textThingFlickering;
        rebuildDisplayData();
        this->uploaded = false;
        flickerTimer = 0.0f;
    } else {
        flickerTimer += deltTime;
    }
}




void GUITextInput::rebuildDisplayData() {

    float xOffset = this->xOffset*700 / GUILwindowWidth;
    float yOffset = this->yOffset*700 / GUILwindowHeight;

    displayData.clear();
    glDeleteBuffers(1, &vbo);
    glGenBuffers(1, &vbo);

    GUITextureFace leftEnd(0,2);
    GUITextureFace rightEnd(1,2);

    GUITextureFace textThing(2,2);

    GUITextureFace middle(0,2);
    middle.bl.x += guiTextureWidth/2;
    middle.tl.x += guiTextureWidth/2;
    
    float lettersCount = std::strlen(label.c_str());
    float unitWidth = (32.0f/GUILwindowWidth)*2;
    float unitHeight = (32.0f/GUILwindowHeight)*2;

    float letHeight = (32.0f/GUILwindowHeight);
    float letWidth = (32.0f/GUILwindowWidth);

    float totwid = unitWidth * 2 + letWidth * lettersCount;
    float tothei = unitHeight;
    float totletwid = letWidth * lettersCount;


    float manWid = (manualWidth * 1280.0f) / GUILwindowWidth;

    totwid = std::max(manWid, totwid);

    glm::vec2 leftStart(-totwid/2 + xOffset, -tothei/2 + yOffset);
    glm::vec2 middleStart(leftStart.x + unitWidth, leftStart.y);
    glm::vec2 rightStart(totwid/2 - unitWidth + xOffset, leftStart.y);

    glm::vec2 letterStart(leftStart.x + letWidth + xOffset, -letHeight/2 + yOffset);

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

    });
    if(textThingFlickering) {
        displayData.insert(displayData.end(), {

            totletwid + (unitWidth/3.0f) + leftStart.x, leftStart.y,                       textThing.bl.x, textThing.bl.y,   elementID,
            totletwid + (unitWidth/3.0f)+ leftStart.x, leftStart.y+unitHeight,             textThing.tl.x, textThing.tl.y,   elementID,
            totletwid + (unitWidth/3.0f)+ leftStart.x+unitWidth, leftStart.y+unitHeight,   textThing.tr.x, textThing.tr.y,   elementID,
        
            totletwid + (unitWidth/3.0f)+ leftStart.x+unitWidth, leftStart.y+unitHeight,   textThing.tr.x, textThing.tr.y,   elementID,
            totletwid + (unitWidth/3.0f)+ leftStart.x+unitWidth, leftStart.y,             textThing.br.x, textThing.br.y,   elementID,
            totletwid + (unitWidth/3.0f)+ leftStart.x, leftStart.y,                       textThing.bl.x, textThing.bl.y,   elementID,

        });
    }

    displayData.insert(displayData.end(), {
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