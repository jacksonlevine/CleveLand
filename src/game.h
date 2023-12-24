#ifndef GAME_H
#define GAME_H

#include <gl/glew.h>
#include "GLFW/glfw3.h"
#include <memory>
#include "shader.h"

class Camera3D;

class Game {
public:
    double deltaTime;
    int windowWidth;
    int windowHeight;
    double mouseSensitivity;

    bool focused;

    GLFWwindow *window;
    Camera3D *camera;

    std::unique_ptr<Shader> menuShader;
    std::unique_ptr<Shader> worldShader;

    void initializeShaders();

    void updateTime();
    void runStep();

    void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    void mouseCallback(GLFWwindow *window, double xpos, double ypos);
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void setFocused(bool focused);

    Game();
private:
    double lastFrame;
};
#endif