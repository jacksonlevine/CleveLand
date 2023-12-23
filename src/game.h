#ifndef GAME_H
#define GAME_H
#include "GLFW/glfw3.h"
#include "camera.h"

class Game {
public:
    double deltaTime;
    int windowWidth;
    int windowHeight;
    double mouseSensitivity;

    GLFWwindow *window;
    Camera3D camera;

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