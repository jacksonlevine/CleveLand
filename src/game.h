#ifndef GAME_H
#define GAME_H



#ifndef MY_GL_INCLUDE
#define MY_GL_INCLUDE
#include <gl/glew.h>
#endif


#ifndef MY_GLFW_INCLUDE
#define MY_GLFW_INCLUDE
#include "GLFW/glfw3.h"
#endif


#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "shader.h"
#include <entt/entt.hpp>
#include "gui/guielement.h"
#include "game/voxelworld.h"
#include "util/meshcomponent.h"
#include "util/raycast.h"
#include "util/collisioncage.h"
#include "gui/hud.h"
#include <random>
#include "util/particle.h"
#include "util/blockinfo.h"
#include "game/specialblocks/door.h"
#include "util/settings.h"
#include "game/specialblocks/chest.h"
#include "game/specialblocks/ladder.h"
#include "network.h"
#include <boost/asio.hpp>
#include "soundfxsystem.h"
#include "util/playergeo.h"
#include "chat/chatstuff.h"
#include "util/cameraposition.h"
#include "mobtype.h"
#include "game/signwords.h"

extern std::string VERSIONSTRING;

extern SoundFXSystem sfs;

extern bool ONSTAIR;

extern float YCAMOFFSET;

class Camera3D;






class Game {
public:

    inline static float introTextAlpha = 0.0f;
    inline static float introTextTimer = 0.0f;


    std::map<std::string, GUIElement*> updateThese;
    std::atomic<float> camRot;

    void updateCamRot();

    boost::asio::io_context io_context;
    TCPClient *client;

    std::thread headCoveredLoop;
    inline static std::atomic<bool> shouldRunHeadCoveredLoop = false;
    inline static std::atomic<bool> headCovered = false;

    inline static float stepSoundTimer = 0.0f;
    inline static float footstepInterval = 0.4f;

    void playFootstepSound();

    void footstepTimer();


    inline static int viewDistance = 5;

    inline static float underWaterView = 0.0f;
    inline static bool inWater = false;

    double deltaTime;
    float averageDeltaTime;
    int windowWidth;
    int windowHeight;
    double mouseSensitivity;

    bool focused;
    inline static bool inGame = false;

    inline static float GRAV = 6.5f;

    GLFWwindow *window;
    Camera3D *camera;
    entt::registry registry;

    Hud* hud;

    std::unique_ptr<Shader> menuShader;
    std::unique_ptr<Shader> worldShader;
    std::unique_ptr<Shader> wireFrameShader;
    std::unique_ptr<Shader> billBoardShader;
    std::unique_ptr<Shader> planetBillboardShader;
    std::unique_ptr<Shader> blockOverlayShader;
    std::unique_ptr<Shader> playerShader;

    void drawCelestialBodies();

    
    GLuint menuTexture;
    GLuint menuBackgroundTexture;
    GLuint splashTexture;
    GLuint splashTexture2;
    GLuint logoTexture;

    GLuint mobsTexture;

    unsigned char *worldTexturePixels;

    GLuint worldTexture;

    VoxelWorld voxelWorld;

    inline static float mousedOverElement = 0.0f;
    inline static float clickedOnElement = 0.0f;

    inline static float rightClickCooldown = 0.1f;
    inline static float rightClickTimear = 0.0f;

    inline static float initialTimer = 0.0f;

    inline static std::vector<GUIElement*> *currentGuiButtons = nullptr;

    std::string currentSingleplayerWorldPath;


    std::vector<Particle> particleDisplayData;
    inline static bool particlesUploaded = false;


    inline static GLuint playersVAO = 0;

    inline static GLuint basePlayerVBO = 0;


    std::vector<PlayerGeo> playerDisplayData;
    inline static bool playerGeoUploaded = false;

    void initializeShaders();
    void initializeTextures();
    void updateTime();
    void runStep();
    void draw();

    void goToMainMenu();
    void goToSingleplayerWorldsMenu();
    void goToMultiplayerWorldsMenu();

    void displayEscapeMenu();

    void goToSingleplayerWorld(const char *worldname);
    void goToMultiplayerWorld();
    void exitMultiplayer();

    void loadOrCreateSaveGame(const char* path);
    void saveGame(const char* path);

    void stepChunkDraw();
    void stepTextureAnim();
    void stepMovementAndPhysics();

    void bindMenuGeometry(GLuint vbo, const float *data, size_t dataSize);
    void bindMenuGeometryNoUpload(GLuint vbo);
    void bindWorldGeometry(GLuint vbov, GLuint vbouv, const float *vdata, const float *uvdata, size_t vsize, size_t uvsize);
    void bindWorldGeometryNoUpload(GLuint vbov, GLuint vbouv);
     void bindWireFrameGeometry(GLuint vbov, const float *vdata,  size_t vsize);
    void bindWireFrameGeometryNoUpload(GLuint vbov);
    void bindBillBoardGeometry(GLuint billposvbo, std::vector<Particle> &billinstances);
    void bindBillBoardGeometryNoUpload(GLuint billposvbo);

    void bindPlayerGeometry(GLuint billposvbo, std::vector<PlayerGeo> &billinstances, MobType type);
    void bindPlayerGeometryNoUpload(GLuint billposvbo);

    void drawPlayers(MobType mt);

    void checkAboveHeadThreadFunction();
    inline static glm::vec3 blockOverlayCoord = glm::vec3(0,0,0);
    inline static bool blockOverlayShowing = false;
    inline static float necessaryBlockBreakingTime = 1.0f;
    inline static float blockBreakingTimer = 0.0f;

    inline static GLuint BOVAO = 0;
    inline static GLuint bovbo = 0;

    void bindBlockOverlayGeometry();
    void bindBlockOverlayGeometryNoUpload();

    void drawBlockOverlay();

    void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    void mouseCallback(GLFWwindow *window, double xpos, double ypos);
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void setFocused(bool focused);

    void drawSky(float top_r, float top_g, float top_b, float top_a,
    float bot_r, float bot_g, float bot_b, float bot_a, float cameraPitch);

    std::function<void (int, int, int, uint32_t)>* mpBlockSetFunc;

    void castBreakRay();
    void castPlaceRay();

    std::function<void()> splashFunc;
    std::function<void()> normalFunc;
    std::function<void()>* loopFunc;

    void drawSplashScreen();

    inline static GLuint VAO = 0;
    inline static GLuint VAO2 = 0;

    CollisionCage collCage;
    BoundingBox user;

    bool grounded;

    bool mainMenu = false;

    inline static float ambientBrightnessMult = 1.0f;

    inline static glm::vec3 currentSelectCube = glm::vec3(0.0f, 0.0f, 0.0f);
    inline static float displayingSelectCube = 0.0f;

    inline static GLuint billqvbo = 0;

    inline static glm::vec3 skyColor = glm::vec3(0.5f, 0.7f, 1.0f);


    void updateAndDrawSelectCube();

    void changeViewDistance(int newValue);

    void getAverageDelta();

    void drawParticles();
    float determineFloorBelowHere(glm::vec3 here, BlockCoord goingAway);

    std::vector<glm::vec3> randomSpotsAroundCube(const glm::vec3& center, int count, float spread);
    void blockBreakParticles(BlockCoord here, int count);

    void splashyParticles(BlockCoord here, int count);
    void cleanUpParticleDisplayData();

    void runPeriodicTick();
    
    void displayLoadScreen(const char *message, float progress, bool inMainLoop);
  
    void drawBackgroundImage();

    void goToUsernameScreen();

    inline static int initialChunksRendered = 0;
    inline static bool loadRendering = true;

    inline static unsigned int selectedBlockID = 1;

    void drawSelectedBlock();

    void goToConfirmDeleteWorld(int worldNum);

    void goToSettingsMenu();

    void goToSignTypeMenu(BlockCoord signPos);
    void renderText(const std::string& message, float x, float y, float scale, float alpha);

    inline static bool changingViewDistance = false;

    inline static Settings settings;

    inline static void saveSettings() {
        std::vector<Setting> sets = {
            Setting{std::string("viewDistance"), std::to_string(viewDistance)},
            Setting{std::string("serverIp"), TYPED_IN_SERVER_IP}
        };
        settings.saveSettings(sets);
    }

   

    inline static int noHud = 0;


    inline static glm::vec3 leanSpot = glm::vec3(0,0,0);
    inline static int leaning = 0;

    inline static float timeOfDay = 0.0f;
    inline static float dayLength = 30.0f;

    inline static float sunsetFactor = 0.0f;
    inline static float sunriseFactor = 0.0f;

    inline static bool inClimbable = false;



    inline static bool inMultiplayer = false;

    inline void drawTextOnScreen(const char* message)
    {
        std::cout <<"PRINT SOMETHING " << std::endl;

        std::cout << "ABout to make vector \n";
        std::vector<float> displayData;
        std::cout << "Made vector \n";

        std::cout << "ABout to get window width height \n";
        float letHeight = (32.0f/ windowHeight);
        float letWidth = (32.0f/ windowWidth);

        std::cout << "Got window width height \n";

        std::cout << "ABout to get strlen \n";
        float lettersCount = std::strlen(message);
        std::cout << "Got strlen " << std::to_string(lettersCount) << " \n";


        float totletwid = letWidth * lettersCount;
        glm::vec2 letterStart(-totletwid/2, -letHeight/2 + 0.2f);

        GlyphFace glyph;
        std::cout << "ABout to  build disp data \n";
        for(int i = 0; i < lettersCount; i++) {
            glyph.setCharCode(static_cast<int>(message[i]));
            glm::vec2 thisLetterStart(letterStart.x + i*letWidth, letterStart.y);
            displayData.insert(displayData.end(), {
                thisLetterStart.x, thisLetterStart.y,                     glyph.bl.x, glyph.bl.y, -1.0f,
                thisLetterStart.x, thisLetterStart.y+letHeight,           glyph.tl.x, glyph.tl.y, -1.0f,
                thisLetterStart.x+letWidth, thisLetterStart.y+letHeight, glyph.tr.x, glyph.tr.y, -1.0f,

                thisLetterStart.x+letWidth, thisLetterStart.y+letHeight, glyph.tr.x, glyph.tr.y, -1.0f,
                thisLetterStart.x+letWidth, thisLetterStart.y,           glyph.br.x, glyph.br.y, -1.0f,
                thisLetterStart.x, thisLetterStart.y,                     glyph.bl.x, glyph.bl.y, -1.0f
            });
        }

        std::cout << "ABout to buffer \n";
        static GLuint vbo2 = 0;
        if(vbo2 == 0) {
            glGenBuffers(1, &vbo2);
        }
        std::cout << "ABout to gbind \n";
        Game::bindMenuGeometry(vbo2,
        displayData.data(),
        displayData.size());
        std::cout << "ABout to tex and draw \n";
        glBindTexture(GL_TEXTURE_2D, menuTexture);
        glDrawArrays(GL_TRIANGLES, 0, displayData.size()/5);

    }

    Game();
private:
    double lastFrame;
};






#endif