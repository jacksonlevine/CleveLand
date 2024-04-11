#ifndef HUD_H
#define HUD_H

#include <vector>
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "../util/guitextureface.h"

struct Hud {

    GLuint vbo;
    std::vector<float> displayData;
    bool uploaded = false;
    inline static int windowWidth = 1280;
    inline static int windowHeight = 720;
    Hud();
    void rebuildDisplayData();
};


#endif