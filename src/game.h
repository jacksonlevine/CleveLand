#ifndef GAME_H
#define GAME_H

#include <gl/glew.h>
#include "GLFW/glfw3.h"
#include <memory>
#include "shader.h"
#include <entt/entt.hpp>
#include <thread>
#include <mutex>
#include "gui/guielement.h"

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
    entt::registry registry;

    std::unique_ptr<Shader> menuShader;
    std::unique_ptr<Shader> worldShader;
    
    GLuint menuTexture;
    GLuint worldTexture;

    std::thread chunkUpdateThread;
    std::mutex meshQueueMutex;

    inline static float mousedOverElement = 0.0f;
    inline static float clickedOnElement = 0.0f;

    inline static std::vector<GUIButton> *currentGuiButtons = nullptr;

    void initializeShaders();
    void initializeTextures();
    void updateTime();
    void runStep();
    void draw();

    void goToTestMenu();
    void goToOtherTestMenu();
    void closeTestMenu();

    void bindMenuGeometry(GLuint vbo, const float *data, size_t dataSize);
    void bindMenuGeometryNoUpload(GLuint vbo);
    void bindWorldGeometry(GLuint vbov, GLuint vbouv, const float *vdata, const float *uvdata, size_t vsize, size_t uvsize);
    void bindWorldGeometryNoUpload(GLuint vbov, GLuint vbouv);

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