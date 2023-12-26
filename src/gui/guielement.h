#ifndef GUIELEMENT_H
#define GUIELEMENT_H
#include "glyphface.h"
#include <vector>
#include "../util/textureface.h"
#include <gl/glew.h>
#include <GLFW/glfw3.h>
class GUIButton {
public:
    glm::vec2 screenPos;
    float screenWidth;
    float screenHeight;
    std::vector<float> displayData;
    float elementID;

    GLuint vbo;
    bool uploaded;

    GUIButton(float xOffset, float yOffset, const char *label, float manualWidth, float elementID,
    int windowWidth, int windowHeight);
    void rebuildDisplayData(int windowWidth, int windowHeight);
private:
    float xOffset;
    float yOffset;
    const char *label;
    float manualWidth;
};

#endif