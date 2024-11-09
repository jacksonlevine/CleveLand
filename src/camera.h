#ifndef CAMERA_H
#define CAMERA_H

#include "game.h"



class Camera3D {
public:
    float yaw;
    float pitch;
    float fov;
    glm::vec3 direction;
    glm::vec3 position;
    glm::vec3 right{};
    glm::vec3 up{};

    glm::mat4 model{};
    glm::mat4 projection{};
    glm::mat4 view{};
    glm::mat4 mvp{};

    glm::vec3 velocity{};


    inline static int forwardPressed = false;
    inline static int leftPressed = false;
    inline static int rightPressed = false;
    inline static int backPressed = false;
    inline static int upPressed = false;
    inline static int downPressed = false;


    inline static int forwardKey = GLFW_KEY_W;
    inline static int leftKey = GLFW_KEY_A;
    inline static int rightKey = GLFW_KEY_D;
    inline static int backKey = GLFW_KEY_S;
    inline static int upKey = GLFW_KEY_SPACE;
    inline static int downKey = GLFW_KEY_LEFT_SHIFT;

    inline static bool firstMouse = true;

    inline static float speedMulitplier = 3.75f;

    inline static float _far = 1300.0f;
    inline static float _near = 0.01f;

    inline static glm::vec3 initialPosition = glm::vec3(0, 120, 0);
    inline static glm::vec3 initialDirection = glm::vec3(0, 0, 1);
    bool focused;

    explicit Camera3D(Game *gs);

    void mouseCallback(GLFWwindow *window, double xpos, double ypos);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void frameBufferSizeCallback(GLFWwindow *window, int width, int height);

    void setFocused(bool focus);


    void updatePosition();


    glm::vec3 proposePosition();
    glm::vec3 proposeSlowPosition();
    void goToPosition(glm::vec3 pos);
private:
    Game *gs;
};
#endif