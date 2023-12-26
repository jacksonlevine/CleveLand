#ifndef GUIELEMENT_H
#define GUIELEMENT_H
#include "glyphface.h"
#include <vector>
#include "../util/textureface.h"
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <functional>

class GUIButton {
public:
    glm::vec2 screenPos;
    float screenWidth;
    float screenHeight;
    std::vector<float> displayData;
    float elementID;
    GLuint vbo;
    bool uploaded;
    inline static int windowWidth = 1280;
    inline static int windowHeight = 720;

    GUIButton(float xOffset, float yOffset, const char *label, float manualWidth, float elementID,
    std::function<void()> function);

    std::function<void()> myFunction;
    
    void rebuildDisplayData();
private:
    float xOffset;
    float yOffset;
    const char *label;
    float manualWidth;
};

#endif