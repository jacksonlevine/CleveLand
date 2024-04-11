#ifndef GUIELEMENT_H
#define GUIELEMENT_H
#include "glyphface.h"
#include <vector>
#include "../util/guitextureface.h"
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <string>
#include <mutex>

extern int GUILwindowWidth;
extern int GUILwindowHeight;



struct KeyInput {
    bool empty;
    char character;
    bool backspace;
};

extern std::mutex GUIMutex;


class GUIElement {
public:
    GUIElement() = default;
    ~GUIElement() = default;

    virtual void rebuildDisplayData() = 0;
    virtual void update(KeyInput in) = 0;
glm::vec2 screenPos;
        float screenHeight;
        float screenWidth;
    std::vector<float> displayData;
    float elementID;
    GLuint vbo;
    bool uploaded;
    bool textThingFlickering;

    std::function<void()> myFunction;
    std::string label;

protected:
    float xOffset;
    float yOffset;
    float manualWidth;
};

class GUIButton : public GUIElement {
public:
    GUIButton(float xOffset, float yOffset, const char *label, float manualWidth, float elementID,
              std::function<void()> function);

    void rebuildDisplayData() override;
    void update(KeyInput in) override;




protected:
    float xOffset;
    float yOffset;
    float manualWidth;
};

class GUITextInput : public GUIElement {
public:
    GUITextInput(float xOffset, float yOffset, const char *label, float manualWidth, float elementID,
                 std::function<void()> function, std::string& storage);

    void rebuildDisplayData() override;
    void update(KeyInput in) override;



protected:
    std::string& storage;
    float xOffset;
    float yOffset;
    float manualWidth;
};

void rebuildGUILDisplayData(GUIElement* guil);
void updateGUIL(GUIElement* guil, KeyInput in);
#endif