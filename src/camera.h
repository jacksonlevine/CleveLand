#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera3D;

#include "game.h"

class Camera3D {
public:
    double yaw;
    double pitch;
    double fov;
    glm::vec3 direction;
    glm::vec3 position;
    glm::vec3 right;
    glm::vec3 up;

    glm::mat4 model;
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 mvp;

    glm::vec3 velocity;

    inline static int forwardKey = GLFW_KEY_W;
    inline static int leftKey = GLFW_KEY_A;
    inline static int rightKey = GLFW_KEY_D;
    inline static int backKey = GLFW_KEY_S;
    inline static float speedMulitplier = 2.0;

    inline static double far = 1500.0;
    inline static double near = 0.01;

    bool focused;

    Camera3D(Game *gs);

    void mouseCallback(GLFWwindow *window, double xpos, double ypos);
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void frameBufferSizeCallback(GLFWwindow *window, int width, int height);

    void setFocused(bool focus);
    void updatePosition();
private:
    Game *gs;
};
#endif