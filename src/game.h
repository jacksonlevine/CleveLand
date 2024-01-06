#ifndef GAME_H
#define GAME_H

#include <gl/glew.h>
#include "GLFW/glfw3.h"
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "shader.h"
#include <entt/entt.hpp>
#include "gui/guielement.h"
#include "game/voxelworld.h"
#include "util/meshcomponent.h"
#include "util/raycast.h"
#include "util/collisioncage.h"

#define GRAV 6.0

class Camera3D;

class Game {
public:
    double deltaTime;
    int windowWidth;
    int windowHeight;
    double mouseSensitivity;

    bool focused;
    inline static bool inGame = false;

    GLFWwindow *window;
    Camera3D *camera;
    entt::registry registry;

    std::unique_ptr<Shader> menuShader;
    std::unique_ptr<Shader> worldShader;
    
    GLuint menuTexture;
    GLuint menuBackgroundTexture;
    GLuint splashTexture;
    GLuint logoTexture;

    unsigned char *worldTexturePixels;

    GLuint worldTexture;

    VoxelWorld voxelWorld;

    inline static float mousedOverElement = 0.0f;
    inline static float clickedOnElement = 0.0f;

    inline static std::vector<GUIButton> *currentGuiButtons = nullptr;

    std::string currentSingleplayerWorldPath;

    void initializeShaders();
    void initializeTextures();
    void updateTime();
    void runStep();
    void draw();

    void goToMainMenu();
    void goToSingleplayerWorldsMenu();

    void displayEscapeMenu();

    void goToSingleplayerWorld(const char *worldname);

    void loadOrCreateSaveGame(const char* path);
    void saveGame(const char* path);

    void stepChunkDraw();
    void stepTextureAnim();

    void bindMenuGeometry(GLuint vbo, const float *data, size_t dataSize);
    void bindMenuGeometryNoUpload(GLuint vbo);
    void bindWorldGeometry(GLuint vbov, GLuint vbouv, const float *vdata, const float *uvdata, size_t vsize, size_t uvsize);
    void bindWorldGeometryNoUpload(GLuint vbov, GLuint vbouv);

    void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    void mouseCallback(GLFWwindow *window, double xpos, double ypos);
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void setFocused(bool focused);

    void drawSky(float top_r, float top_g, float top_b, float top_a,
    float bot_r, float bot_g, float bot_b, float bot_a, float cameraPitch);

    void castBreakRay();

    std::function<void()> splashFunc;
    std::function<void()> normalFunc;
    std::function<void()>* loopFunc;

    void drawSplashScreen();

    inline static GLuint VAO = 0;

    CollisionCage collCage;
    BoundingBox user;

    bool grounded;

    bool mainMenu = false;

    Game();
private:
    double lastFrame;
};
#endif