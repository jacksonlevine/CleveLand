#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Game::Game() : lastFrame(0), focused(false), camera(nullptr),
collCage([this](BlockCoord b){
    return voxelWorld.blockAt(b) != 0;
}),
user(glm::vec3(0,0,0), glm::vec3(0,0,0)),
grounded(true)
{

    windowWidth = 1280;
    windowHeight = 720;
    camera = new Camera3D(this);
    mouseSensitivity = 0.1;

    glfwInit();
    window = glfwCreateWindow(windowWidth, windowHeight, "Untitled game", NULL, NULL);
    glfwMakeContextCurrent(window);
    this->setFocused(true);
    glewInit();
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
    initializeShaders();
    initializeTextures();
    hud = new Hud();
    hud->rebuildDisplayData();
    //voxelWorld.populateChunksAndGeometryStores(registry);

    goToMainMenu();

    normalFunc = [this]() {

            stepMovementAndPhysics();

            
            draw();
            

            glfwPollEvents();
            updateTime();

            if(inGame) {
                voxelWorld.runStep(deltaTime);

                static float textureAnimInterval = 0.1f;
                static float textureAnimTimer = 0.0f;
                if(textureAnimTimer > textureAnimInterval) {
                    textureAnimTimer = 0.0f;
                    stepTextureAnim();
                } else {
                    textureAnimTimer += deltaTime;
                }
            }
    };

    splashFunc = [this](){
        static float timer = 0.0f;
        drawSplashScreen();
        glfwPollEvents();
        updateTime();
        if(timer > 5.0f) {
            loopFunc = &normalFunc;
        } else {
            timer += deltaTime;
        }
    };

    loopFunc = &splashFunc;
}

void Game::stepMovementAndPhysics() {
            static float currentJumpY = 0.0f;
            float allowableJumpHeight = 0.5f;
            static bool jumpingUp = false;

            static float timeFallingScalar = 1.0f;

            if(!grounded) {
                timeFallingScalar += deltaTime*2.0f;
            } else {
                timeFallingScalar = 1.0f;
            }


            glm::vec3 collCageCenter = camera->position + glm::vec3(0, -1.0, 0);
            collCage.update_readings(collCageCenter);

            if(std::find(collCage.solid.begin(), collCage.solid.end(), FLOOR) == collCage.solid.end())
            {
                grounded = false;
            }

            if(!grounded && !jumpingUp /*&& jumpTimer <= 0.0f*/)
            {
                camera->velocity += glm::vec3(0.0, -GRAV*timeFallingScalar*deltaTime, 0.0);
            }

            if(jumpingUp) {
                if(camera->position.y < currentJumpY + allowableJumpHeight) {
                    camera->velocity += glm::vec3(0.0f, (currentJumpY+allowableJumpHeight) - camera->position.y, 0.0f);
                } else {
                    jumpingUp = false;
                }
            }

            // if(jumpTimer > 0.0f) {
            //     jumpTimer = std::max(0.0, jumpTimer - deltaTime);
            // }

            if(camera->upPressed && grounded)
            {
                //camera->velocity += glm::vec3(0.0, 100.0*deltaTime, 0.0);
                grounded = false;
                //jumpTimer = deltaTime*10.0f;
                currentJumpY = camera->position.y;
                jumpingUp = true;
                camera->upPressed = 0;
            }



            

            glm::vec3 proposedPos = camera->proposePosition();

            std::vector<glm::vec3> corrections_made;

            user.set_center(proposedPos + glm::vec3(0.0, -0.5, 0.0), 0.85f, 0.2f);
            collCage.update_colliding(user);

            if(collCage.colliding.size() > 0) {
                for(Side side : collCage.colliding) {
                    if(std::find(corrections_made.begin(), corrections_made.end(), CollisionCage::normals[side]) == corrections_made.end())
                    {
                        proposedPos += CollisionCage::normals[side] * (float)collCage.penetration[side];
                        corrections_made.push_back(CollisionCage::normals[side]);
                    }

                    if(side == FLOOR)
                    {
                        grounded = true;
                    }
                    if(side == ROOF)
                    {
                        jumpingUp = false;
                        grounded = false;
                    }
                }
            }
            camera->goToPosition(proposedPos);
}

void Game::drawSplashScreen() {

    if(VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);


    float splashImageWidth = 400;



    glm::vec2 splashLowerLeft(-splashImageWidth/windowWidth, -splashImageWidth/windowHeight);
    float relHeight = splashImageWidth/(windowHeight/2);
    float relWidth = splashImageWidth/(windowWidth/2);


    std::vector<float> splashDisplayData = {
        splashLowerLeft.x, splashLowerLeft.y,                    0.0f, 1.0f,   -1.0f,
        splashLowerLeft.x, splashLowerLeft.y+relHeight,          0.0f, 0.0f,   -1.0f,
        splashLowerLeft.x+relWidth, splashLowerLeft.y+relHeight, 1.0f, 0.0f,   -1.0f,

        splashLowerLeft.x+relWidth, splashLowerLeft.y+relHeight, 1.0f, 0.0f,   -1.0f,
        splashLowerLeft.x+relWidth, splashLowerLeft.y,           1.0f, 1.0f,   -1.0f,
        splashLowerLeft.x, splashLowerLeft.y,                    0.0f, 1.0f,   -1.0f
    };



    glUseProgram(menuShader->shaderID);

   
    glBindTexture(GL_TEXTURE_2D, splashTexture);


    static GLuint vbo = 0;
    if(vbo == 0) {
        glGenBuffers(1, &vbo);
    }

        bindMenuGeometry(vbo, 
        splashDisplayData.data(),
        splashDisplayData.size());


    glDrawArrays(GL_TRIANGLES, 0, splashDisplayData.size()/5);

    

    glfwSwapBuffers(window);
}

void Game::draw() {
    
    if(VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    }


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.639, 0.71, 1.0, 0.5);


    drawSky(0.0f, 0.0f, 1.0f, 1.0f,    1.2f, 1.2f, 1.8f, 1.0f, camera->pitch);

    
    if(currentGuiButtons != nullptr) {
        glUseProgram(menuShader->shaderID);
        glBindTexture(GL_TEXTURE_2D, menuTexture);

        mousedOverElement = 0.0f;

        for(GUIButton& button : *currentGuiButtons) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            if(xpos > button.screenPos.x * windowWidth &&
            xpos < (button.screenPos.x + button.screenWidth) * windowWidth &&
            ypos > button.screenPos.y * windowHeight &&
            ypos < (button.screenPos.y + button.screenHeight) * windowHeight)
            {
                mousedOverElement = button.elementID;
            }

            if(!button.uploaded) {
                bindMenuGeometry(button.vbo, button.displayData.data(), button.displayData.size());
                button.uploaded = true;
            } else {
                bindMenuGeometryNoUpload(button.vbo);
            }
            glDrawArrays(GL_TRIANGLES, 0, button.displayData.size() / 5);
        }

        GLuint moeLocation = glGetUniformLocation(menuShader->shaderID, "mousedOverElement");
        glUniform1f(moeLocation, mousedOverElement);
        GLuint coeLocation = glGetUniformLocation(menuShader->shaderID, "clickedOnElement");
        glUniform1f(coeLocation, clickedOnElement);



        if(mainMenu) {










            float logoImageWidth = 600;



            glm::vec2 logoLowerLeft(-logoImageWidth/windowWidth, -logoImageWidth/windowHeight);
            float relHeight = logoImageWidth/(windowHeight/2);
            float relWidth = logoImageWidth/(windowWidth/2);

            float shiftUp = 0.5f;


            std::vector<float> logoDisplayData = {
                logoLowerLeft.x, logoLowerLeft.y + shiftUp,                    0.0f, 1.0f,   -1.0f,
                logoLowerLeft.x, logoLowerLeft.y+relHeight+ shiftUp,          0.0f, 0.0f,   -1.0f,
                logoLowerLeft.x+relWidth, logoLowerLeft.y+relHeight+ shiftUp, 1.0f, 0.0f,   -1.0f,

                logoLowerLeft.x+relWidth, logoLowerLeft.y+relHeight+ shiftUp, 1.0f, 0.0f,   -1.0f,
                logoLowerLeft.x+relWidth, logoLowerLeft.y+ shiftUp,           1.0f, 1.0f,   -1.0f,
                logoLowerLeft.x, logoLowerLeft.y+ shiftUp,                    0.0f, 1.0f,   -1.0f
            };

        
            glBindTexture(GL_TEXTURE_2D, logoTexture);


            static GLuint vbo = 0;
            if(vbo == 0) {
                glGenBuffers(1, &vbo);
            }

                bindMenuGeometry(vbo, 
                logoDisplayData.data(),
                logoDisplayData.size());


            glDrawArrays(GL_TRIANGLES, 0, logoDisplayData.size()/5);

            




















        }

    }

    if(!inGame) {
        glUseProgram(menuShader->shaderID);
        glBindTexture(GL_TEXTURE_2D, menuBackgroundTexture);
        static GLuint backgroundVbo = 0;
        if(backgroundVbo == 0) {
            glGenBuffers(1, &backgroundVbo);
        }
        static int lastWindowWidth = 0;
        static int lastWindowHeight = 0;

        static std::vector<float> backgroundImageData;

        if(lastWindowWidth != windowWidth || lastWindowHeight != windowHeight) {
            lastWindowWidth = windowWidth;
            lastWindowHeight = windowHeight;
            backgroundImageData.clear();
            //X, Y, U, V, ElementID
            backgroundImageData.insert(backgroundImageData.end(), {
                -1.0, -1.0,   0.0, 0.0,                                -1.0f,
                -1.0, 1.0,    0.0, windowHeight/160.0f,                -1.0f,
                1.0, 1.0,     windowWidth/160.0f, windowHeight/160.0f, -1.0f,

                1.0, 1.0,     windowWidth/160.0f, windowHeight/160.0f, -1.0f,
                1.0, -1.0,    windowWidth/160.0f, 0.0,                 -1.0f,
                -1.0, -1.0,   0.0, 0.0,                                -1.0f
            });
            bindMenuGeometry(backgroundVbo, backgroundImageData.data(), backgroundImageData.size());
        } else {
            bindMenuGeometryNoUpload(backgroundVbo);
        }
        glDrawArrays(GL_TRIANGLES, 0, backgroundImageData.size() / 5);
    }

    if(inGame) {
        glUseProgram(menuShader->shaderID);
        glBindTexture(GL_TEXTURE_2D, menuTexture);

        if(hud->uploaded) {
            bindMenuGeometryNoUpload(hud->vbo);
        } else {
            bindMenuGeometry(hud->vbo,
            hud->displayData.data(),
            hud->displayData.size());
        }
        glDrawArrays(GL_TRIANGLES, 0, hud->displayData.size()/5);

        stepChunkDraw();
        GLuint ambBrightMultLoc = glGetUniformLocation(worldShader->shaderID, "ambientBrightMult");

        glUniform1f(ambBrightMultLoc, ambientBrightnessMult);
        updateAndDrawSelectCube();
    }


    glfwSwapBuffers(window);
}

void Game::stepChunkDraw() {
    
    glUseProgram(worldShader->shaderID);
    glBindTexture(GL_TEXTURE_2D, worldTexture);

    // const float* matrixData = glm::value_ptr(camera->mvp);

    // std::cout << "Matrix:" << std::endl;
    // for (int i = 0; i < 4; ++i) {
    //     for (int j = 0; j < 4; ++j) {
    //         std::cout << matrixData[i + j * 4] << "\t";
    //     }
    //     std::cout << std::endl;
    // }

    GLuint mvp_loc = glGetUniformLocation(worldShader->shaderID, "mvp");
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(camera->mvp));

    GLuint cam_pos_loc = glGetUniformLocation(worldShader->shaderID, "camPos");
    glUniform3f(cam_pos_loc, camera->position.x, camera->position.y, camera->position.z);

    voxelWorld.cameraPosition = camera->position;
    voxelWorld.cameraDirection = camera->direction;
    if(voxelWorld.highPriorityGeometryStoresToRebuild.size() > 0) {

            GeometryStore &geometryStore = voxelWorld.geometryStorePool[voxelWorld.highPriorityGeometryStoresToRebuild.back()];
            if(!registry.all_of<MeshComponent>(geometryStore.me)) {
                MeshComponent m;
                m.length = geometryStore.verts.size();
                bindWorldGeometry(
                    m.vbov,
                    m.vbouv,
                    geometryStore.verts.data(),
                    geometryStore.uvs.data(),
                    geometryStore.verts.size(),
                    geometryStore.uvs.size()
                );
                //Transparent stuff
                m.tlength = geometryStore.tverts.size();
                bindWorldGeometry(
                    m.tvbov,
                    m.tvbouv,
                    geometryStore.tverts.data(),
                    geometryStore.tuvs.data(),
                    geometryStore.tverts.size(),
                    geometryStore.tuvs.size()
                );
                registry.emplace<MeshComponent>(geometryStore.me, m);
            } else {
                MeshComponent &m = registry.get<MeshComponent>(geometryStore.me);
                glDeleteBuffers(1, &m.vbov);
                glDeleteBuffers(1, &m.vbouv);
                glGenBuffers(1, &m.vbov);
                glGenBuffers(1, &m.vbouv);


                m.length = geometryStore.verts.size();
                bindWorldGeometry(
                    m.vbov,
                    m.vbouv,
                    geometryStore.verts.data(),
                    geometryStore.uvs.data(),
                    geometryStore.verts.size(),
                    geometryStore.uvs.size()
                );
                //Transparent stuff
                glDeleteBuffers(1, &m.tvbov);
                glDeleteBuffers(1, &m.tvbouv);
                glGenBuffers(1, &m.tvbov);
                glGenBuffers(1, &m.tvbouv);

                m.tlength = geometryStore.tverts.size();
                bindWorldGeometry(
                    m.tvbov,
                    m.tvbouv,
                    geometryStore.tverts.data(),
                    geometryStore.tuvs.data(),
                    geometryStore.tverts.size(),
                    geometryStore.tuvs.size()
                );
            }
            voxelWorld.highPriorityGeometryStoresToRebuild.pop_back();

    } else {
        if(voxelWorld.geometryStoresToRebuild.size() > 0) {
            if(voxelWorld.meshQueueMutex.try_lock()) {
                GeometryStore &geometryStore = voxelWorld.geometryStorePool[voxelWorld.geometryStoresToRebuild.back()];
                if(!registry.all_of<MeshComponent>(geometryStore.me)) {
                    MeshComponent m;
                    m.length = geometryStore.verts.size();
                    bindWorldGeometry(
                        m.vbov,
                        m.vbouv,
                        geometryStore.verts.data(),
                        geometryStore.uvs.data(),
                        geometryStore.verts.size(),
                        geometryStore.uvs.size()
                    );
                    //Transparent stuff
                    m.tlength = geometryStore.tverts.size();
                    bindWorldGeometry(
                        m.tvbov,
                        m.tvbouv,
                        geometryStore.tverts.data(),
                        geometryStore.tuvs.data(),
                        geometryStore.tverts.size(),
                        geometryStore.tuvs.size()
                    );
                    registry.emplace<MeshComponent>(geometryStore.me, m);
                } else {
                    MeshComponent &m = registry.get<MeshComponent>(geometryStore.me);
                    glDeleteBuffers(1, &m.vbov);
                    glDeleteBuffers(1, &m.vbouv);
                    glGenBuffers(1, &m.vbov);
                    glGenBuffers(1, &m.vbouv);


                    m.length = geometryStore.verts.size();
                    bindWorldGeometry(
                        m.vbov,
                        m.vbouv,
                        geometryStore.verts.data(),
                        geometryStore.uvs.data(),
                        geometryStore.verts.size(),
                        geometryStore.uvs.size()
                    );
                    //Transparent stuff
                    glDeleteBuffers(1, &m.tvbov);
                    glDeleteBuffers(1, &m.tvbouv);
                    glGenBuffers(1, &m.tvbov);
                    glGenBuffers(1, &m.tvbouv);

                    m.tlength = geometryStore.tverts.size();
                    bindWorldGeometry(
                        m.tvbov,
                        m.tvbouv,
                        geometryStore.tverts.data(),
                        geometryStore.tuvs.data(),
                        geometryStore.tverts.size(),
                        geometryStore.tuvs.size()
                    );
                }
                voxelWorld.geometryStoresToRebuild.pop_back();
                voxelWorld.meshQueueMutex.unlock();
            }
        }
    }

    
        auto meshesView = registry.view<MeshComponent>();
    for(const entt::entity e : meshesView) {
        MeshComponent &m = registry.get<MeshComponent>(e);
        //std::cout << "ddrawing a chunk of size " << m.length << "\n";
        bindWorldGeometryNoUpload(
            m.vbov,
            m.vbouv
        );
        glDrawArrays(GL_TRIANGLES, 0, m.length);
    }
    for(const entt::entity e : meshesView) {
        MeshComponent &m = registry.get<MeshComponent>(e);
        //std::cout << "ddrawing a chunk of size " << m.length << "\n";
        bindWorldGeometryNoUpload(
            m.tvbov,
            m.tvbouv
        );
        glDrawArrays(GL_TRIANGLES, 0, m.tlength);
    }


}

void Game::displayEscapeMenu() {
    camera->setFocused(false);
    static std::vector<GUIButton> buttons = {
        GUIButton(0.0f, 0.0f, "Save and exit to main menu", 0.0f, 1.0f, [this](){
            
            inGame = false;
            voxelWorld.runChunkThread = false;
            //voxelWorld.chunkUpdateThread.join();
            saveGame(currentSingleplayerWorldPath.c_str());
            camera->setFocused(false);

            voxelWorld.userDataMap.clear();
            voxelWorld.geometryStoresToRebuild.clear();
            voxelWorld.takenCareOfChunkSpots.clear();
            voxelWorld.geometryStorePool.clear();
            voxelWorld.chunks.clear();


            auto meshesView = registry.view<MeshComponent>();
            for(const entt::entity e : meshesView) {
                MeshComponent &m = registry.get<MeshComponent>(e);
                glDeleteBuffers(1, &m.vbov);
                glDeleteBuffers(1, &m.vbouv);
                glDeleteBuffers(1, &m.tvbov);
                glDeleteBuffers(1, &m.tvbouv);
                registry.erase<MeshComponent>(e);
                registry.destroy(e);
            }

            registry.clear();

            goToMainMenu();
        }),
        GUIButton(0.0f, -0.1f, "Back to game", 0.0f, 2.0f, [this](){
            camera->firstMouse = true;
            camera->setFocused(true);
            currentGuiButtons = nullptr;
        }),
    };
    for(GUIButton &button : buttons) {
        button.rebuildDisplayData();
        button.uploaded = false;
    }
    currentGuiButtons = &buttons;
}

void Game::goToMainMenu() {
    mainMenu = true;
    static std::vector<GUIButton> buttons = {
        GUIButton(0.0f, 0.0f, "Singleplayer", 0.0f, 1.0f, [this](){
            this->goToSingleplayerWorldsMenu();
            mainMenu = false;
        }),
        GUIButton(0.0f, -0.1f, "Quit Game", 0.0f, 2.0f, [this](){
            glfwSetWindowShouldClose(this->window, GLFW_TRUE);
            mainMenu = false;
        }),
    };
    for(GUIButton &button : buttons) {
        button.rebuildDisplayData();
        button.uploaded = false;
    }
    currentGuiButtons = &buttons;
}

void Game::goToSingleplayerWorldsMenu() {
    static std::vector<GUIButton> buttons = {
        GUIButton(0.0f, 0.2f, "World 1", 0.0f, 1.0f, [this](){
            goToSingleplayerWorld("world1");
        }),
        GUIButton(0.0f, 0.1f, "World 2", 0.0f, 2.0f, [this](){
            goToSingleplayerWorld("world2");
        }),
        GUIButton(0.0f, 0.0f, "World 3", 0.0f, 3.0f, [this](){
            goToSingleplayerWorld("world3");
        }),
        GUIButton(0.0f, -0.1f, "World 4", 0.0f, 4.0f, [this](){
            goToSingleplayerWorld("world4");
        }),
        GUIButton(0.0f, -0.2f, "World 5", 0.0f, 5.0f, [this](){
            goToSingleplayerWorld("world5");
        }),
        GUIButton(0.0f, -0.3f, "Back to main menu", 0.0f, 6.0f, [this](){
            this->goToMainMenu();
        }),
    };
    for(GUIButton &button : buttons) {
        button.rebuildDisplayData();
        button.uploaded = false;
    }
    currentGuiButtons = &buttons;
}

void Game::loadOrCreateSaveGame(const char* path) {
    if(voxelWorld.saveExists(path)) {
        voxelWorld.loadWorldFromFile(path);
    } else {
        voxelWorld.seed = time(NULL);
        voxelWorld.saveWorldToFile(path);
    }
    
    std::string playerInfoPath = std::string(path) + "/player.save";
    if(!std::filesystem::exists(playerInfoPath)) {
        std::ofstream playerFile(playerInfoPath, std::ios::trunc);
        if(playerFile.is_open()) {
            playerFile << camera->initialPosition.x << " " << camera->initialPosition.y << " " <<
            camera->initialPosition.z << "\n";

            playerFile << camera->initialDirection.x << " " << camera->initialDirection.y << " " <<
            camera->initialDirection.z << "\n";

            playerFile << "0.0" << " " << "0.0" << "\n";
        } else {
            std::cerr << "Couldn't open player file when initializing. \n";
        }
        playerFile.close();
    }

    glm::vec3 loadedPosition;
    glm::vec3 loadedDirection;
    float loadedYaw = 0.0f;
    float loadedPitch = 0.0f;

    std::ifstream playerFile(playerInfoPath);
    if(playerFile.is_open()) {
        std::string line;
        int lineIndex = 0;
        while(std::getline(playerFile, line)) {
            std::istringstream linestream(line);
            std::string word;
            int localIndex = 0;
            while(linestream >> word) {
                if(lineIndex == 0) {

                    if(localIndex == 0) {
                        loadedPosition.x = std::stof(word);
                    }
                    if(localIndex == 1) {
                        loadedPosition.y = std::stof(word);
                    }
                    if(localIndex == 2) {
                        loadedPosition.z = std::stof(word);
                    }
                }
                if(lineIndex == 1) {

                    if(localIndex == 0) {
                        loadedDirection.x = std::stof(word);
                    }
                    if(localIndex == 1) {
                        loadedDirection.y = std::stof(word);
                    }
                    if(localIndex == 2) {
                        loadedDirection.z = std::stof(word);
                    }
                }
                if(lineIndex == 2) {

                    if(localIndex == 0) {
                        loadedYaw = std::stof(word);
                    }
                    if(localIndex == 1) {
                        loadedPitch = std::stof(word);
                    }
                }
                localIndex++;
            }
            lineIndex++;
        }
    playerFile.close();
    } else {
        std::cerr << "Couldn't open player file when loading. \n";
    }
    camera->yaw = loadedYaw;
    camera->pitch = loadedPitch;
    camera->position = loadedPosition;
    camera->direction = loadedDirection;
    camera->firstMouse = true;
    camera->right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camera->direction));
    camera->up = glm::cross(camera->direction, camera->right);
    camera->updatePosition();
}

void Game::saveGame(const char* path) {
    voxelWorld.saveWorldToFile(path);
    std::string playerInfoPath = std::string(path) + "/player.save";
    std::ofstream playerFile(playerInfoPath, std::ios::trunc);
    if(playerFile.is_open()) {
        playerFile << camera->position.x << " " << camera->position.y << " " <<
        camera->position.z << "\n";

        playerFile << camera->direction.x << " " << camera->direction.y << " " <<
        camera->direction.z << "\n";

        playerFile << camera->yaw << " " << camera->pitch << "\n";
    } else {
        std::cerr << "Couldn't open player file when saving. \n";
    }
    playerFile.close();
}

void Game::goToSingleplayerWorld(const char *worldname) {




    voxelWorld.populateChunksAndGeometryStores(registry);



    currentGuiButtons = nullptr;

    currentSingleplayerWorldPath = std::string("saves/") + std::string(worldname);

    loadOrCreateSaveGame(currentSingleplayerWorldPath.c_str());


    voxelWorld.runChunkThread = true;
    voxelWorld.chunkUpdateThread = std::thread([this](){
        voxelWorld.chunkUpdateThreadFunction();
        });
    voxelWorld.chunkUpdateThread.detach();


    camera->setFocused(true);

    inGame = true;
}

void Game::updateTime() {
    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Game::runStep() {
    (*loopFunc)();
}

void Game::frameBufferSizeCallback(GLFWwindow *window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, windowWidth, windowHeight);
    camera->frameBufferSizeCallback(window, windowWidth, windowHeight);

    GUIButton::windowWidth = windowWidth;
    GUIButton::windowHeight = windowHeight;
    
    if(currentGuiButtons != nullptr) {
        for(GUIButton &button : *currentGuiButtons) {
            button.rebuildDisplayData();
            button.uploaded = false;
        }
    }
    hud->rebuildDisplayData();
    hud->uploaded = false;
}

void Game::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if(!focused) {
        setFocused(true);
        if(inGame) {
            camera->setFocused(true);
        }
    }
    if(action == GLFW_PRESS) {

        if(inGame && camera->focused) {
            if(button == GLFW_MOUSE_BUTTON_LEFT)
                castBreakRay();
            if(button == GLFW_MOUSE_BUTTON_RIGHT)
                castPlaceRay();
        }

        clickedOnElement = mousedOverElement;
    } else {
        if(currentGuiButtons != nullptr) {
            for(auto &button : *currentGuiButtons) {
                if(button.elementID == clickedOnElement) {
                    button.myFunction();
                }
            }
        }
        clickedOnElement = 0.0f;
    }
}

void Game::castBreakRay() {
    RayCastResult rayResult = rayCast(
        voxelWorld.chunkWidth,
        camera->position,
        camera->direction,
        [this](BlockCoord coord){
            return voxelWorld.blockAt(coord) != 0;
        },
        true
    );
    if(rayResult.hit) {
        if(rayResult.chunksToRebuild.size() > 0) {
            if(voxelWorld.userDataMap.find(rayResult.chunksToRebuild.front()) == voxelWorld.userDataMap.end()) {
                voxelWorld.userDataMap.insert_or_assign(rayResult.chunksToRebuild.front(), 
                std::unordered_map<BlockCoord, unsigned int, IntTupHash>());
            }
            voxelWorld.userDataMap.at(rayResult.chunksToRebuild.front()).insert_or_assign(rayResult.blockHit, 0);


                for(ChunkCoord& ccoord : rayResult.chunksToRebuild) {
                    auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(ccoord);
                    if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                        //std::cout << "it's here" << "\n";
                      //  std::cout << "fucking index:" << chunkIt->second.geometryStorePoolIndex << "\n";
                        voxelWorld.rebuildChunk(chunkIt->second, chunkIt->second.position, true);
                    }
                }
        }

    }
}
void Game::castPlaceRay() {
    RayCastResult rayResult = rayCast(
        voxelWorld.chunkWidth,
        camera->position,
        camera->direction,
        [this](BlockCoord coord){
            return voxelWorld.blockAt(coord) != 0;
        },
        false
    );
    if(rayResult.hit) {
        if(rayResult.chunksToRebuild.size() > 0) {
            if(voxelWorld.userDataMap.find(rayResult.chunksToRebuild.front()) == voxelWorld.userDataMap.end()) {
                voxelWorld.userDataMap.insert_or_assign(rayResult.chunksToRebuild.front(), 
                std::unordered_map<BlockCoord, unsigned int, IntTupHash>());
            }

            glm::vec3 blockHit(rayResult.blockHit.x, rayResult.blockHit.y, rayResult.blockHit.z);
            glm::vec3 hitPoint = rayResult.head;

            glm::vec3 diff = hitPoint - blockHit;

            glm::vec3 hitNormal;

            // Determine the primary axis of intersection
            if (abs(diff.x) > abs(diff.y) && abs(diff.x) > abs(diff.z)) {
                // The hit was primarily along the X-axis
                hitNormal = glm::vec3((diff.x > 0) ? 1.0f : -1.0f, 0.0f, 0.0f);
            } else if (abs(diff.y) > abs(diff.x) && abs(diff.y) > abs(diff.z)) {
                // The hit was primarily along the Y-axis
                hitNormal = glm::vec3(0.0f, (diff.y > 0) ? 1.0f : -1.0f, 0.0f);
            } else {
                // The hit was primarily along the Z-axis
                hitNormal = glm::vec3(0.0f, 0.0f, (diff.z > 0) ? 1.0f : -1.0f);
            }

            std::cout << "Place normal: " << hitNormal.x << " " << hitNormal.y << " " << hitNormal.z << "\n";



            BlockCoord placePoint(rayResult.blockHit.x+hitNormal.x, rayResult.blockHit.y+hitNormal.y, rayResult.blockHit.z+hitNormal.z);

            voxelWorld.userDataMap.at(rayResult.chunksToRebuild.front()).insert_or_assign(placePoint, 1);


            ChunkCoord chunkToReb(static_cast<int>(std::floor(static_cast<float>(placePoint.x)/voxelWorld.chunkWidth)),
            static_cast<int>(std::floor(static_cast<float>(placePoint.z)/voxelWorld.chunkWidth)));

                    auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(chunkToReb);
                    if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                        //std::cout << "it's here" << "\n";
                      //  std::cout << "fucking index:" << chunkIt->second.geometryStorePoolIndex << "\n";
                        voxelWorld.rebuildChunk(chunkIt->second, chunkIt->second.position, true);

                     }

        }
    }
}

void Game::mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    if(camera->focused) {
        camera->mouseCallback(window, xpos, ypos);
    }
}

void Game::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(camera->focused) {
        camera->keyCallback(window, key, scancode, action, mods);
    }
    if(key == GLFW_KEY_ESCAPE) {
        if(inGame) {
            displayEscapeMenu();
        }

    }
    if(key == GLFW_KEY_KP_SUBTRACT) {
        ambientBrightnessMult = std::max(ambientBrightnessMult - 0.01f, 0.0f);

    } 
    if(key == GLFW_KEY_KP_ADD) {
        ambientBrightnessMult = std::min(ambientBrightnessMult + 0.01f, 1.0f);

    }
}

void Game::setFocused(bool focused) {
    this->focused = focused;
    if(focused) {
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
            Game* instance = static_cast<Game*>(glfwGetWindowUserPointer(w));
            if (instance) {
                instance->keyCallback(w, key, scancode, action, mods);
            }
        });
        glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
            Game* instance = static_cast<Game*>(glfwGetWindowUserPointer(w));
            if (instance) {
                instance->mouseCallback(w, xpos, ypos);
            }
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
            Game* instance = static_cast<Game*>(glfwGetWindowUserPointer(w));
            if (instance) {
                instance->mouseButtonCallback(w, button, action, mods);
            }
        });
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
            Game* instance = static_cast<Game*>(glfwGetWindowUserPointer(w));
            if (instance) {
                instance->frameBufferSizeCallback(w, width, height);
            }
        });
    } else {
        glfwSetKeyCallback(window, NULL);
        glfwSetCursorPosCallback(window, NULL);
        glfwSetFramebufferSizeCallback(window, NULL);
        camera->firstMouse = true;
    }
}

void Game::updateAndDrawSelectCube() {
    static std::vector<float> faces = {

        -0.501f, -0.501f, -0.501f,  0.501f, -0.501f, -0.501f, // Bottom Face
        0.501f, -0.501f, -0.501f,   0.501f, -0.501f,  0.501f,
        0.501f, -0.501f,  0.501f,  -0.501f, -0.501f,  0.501f,
        -0.501f, -0.501f,  0.501f, -0.501f, -0.501f, -0.501f,

        -0.501f,  0.501f, -0.501f,  0.501f,  0.501f, -0.501f, // Top Face
        0.501f,  0.501f, -0.501f,   0.501f,  0.501f,  0.501f,
        0.501f,  0.501f,  0.501f,  -0.501f,  0.501f,  0.501f,
        -0.501f,  0.501f,  0.501f, -0.501f,  0.501f, -0.501f,

        -0.501f, -0.501f, -0.501f, -0.501f,  0.501f, -0.501f, // Side Edges
        0.501f, -0.501f, -0.501f,   0.501f,  0.501f, -0.501f,
        0.501f, -0.501f,  0.501f,   0.501f,  0.501f,  0.501f,
        -0.501f, -0.501f,  0.501f, -0.501f,  0.501f,  0.501f

    };

    //glDisable(GL_DEPTH_TEST);

    glUseProgram(wireFrameShader->shaderID);

    static GLuint vbo = 0;
    if(vbo == 0) {
        glGenBuffers(1, &vbo);
        bindWireFrameGeometry(vbo, faces.data(), faces.size());

    } else {
        bindWireFrameGeometryNoUpload(vbo);
    }

    GLuint wfTranslationLoc = glGetUniformLocation(wireFrameShader->shaderID, "translation");
    glUniform3f(wfTranslationLoc, currentSelectCube.x, currentSelectCube.y, currentSelectCube.z);
    GLuint mvp_loc = glGetUniformLocation(wireFrameShader->shaderID, "mvp");
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(camera->mvp));
    GLuint displayingLoc = glGetUniformLocation(wireFrameShader->shaderID, "displaying");
    glUniform1f(displayingLoc, displayingSelectCube);

    RayCastResult result = rayCast(voxelWorld.chunkWidth,
    camera->position,
    camera->direction,
    [this](BlockCoord coord){
            return voxelWorld.blockAt(coord) != 0;
    },
    false
    );
    if(!result.hit) {
        displayingSelectCube = 0.0f;
    } else {
        displayingSelectCube = 1.0f;
        currentSelectCube = glm::vec3(static_cast<float>(result.blockHit.x),
        static_cast<float>(result.blockHit.y),
        static_cast<float>(result.blockHit.z));
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glDrawArrays(GL_LINES, 0, faces.size()/3);

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    //glEnable(GL_DEPTH_TEST);
}

void Game::initializeShaders() {
    menuShader = std::make_unique<Shader>(
        R"glsl(
            #version 330 core
            layout (location = 0) in vec2 pos;
            layout (location = 1) in vec2 texcoord;
            layout (location = 2) in float elementid;

            out vec2 TexCoord;
            out float elementID;

            void main()
            {
                gl_Position = vec4(pos, 0.0, 1.0);
                TexCoord = texcoord;
                elementID = elementid;
            }
        )glsl",
        R"glsl(
            #version 330 core
            out vec4 FragColor;
            in vec2 TexCoord;
            in float elementID;
            uniform sampler2D ourTexture;
            uniform float mousedOverElement;
            uniform float clickedOnElement;
            void main() {
                FragColor = texture(ourTexture, TexCoord);
                if(FragColor.a < 1.0) {
                    discard;
                }
                if(clickedOnElement == elementID) {
                    FragColor = vec4(vec3(1.0, 1.0, 1.0) - FragColor.rgb, 1.0);
                } else if(mousedOverElement == elementID) {
                    FragColor = FragColor + vec4(0.3, 0.3, 0.3, 0.0);
                }
            }
        )glsl",
        "menuShader"
    );
    worldShader = std::make_unique<Shader>(
        R"glsl(
            #version 330 core
            layout (location = 0) in vec3 position;
            layout (location = 1) in float blockBright;
            layout (location = 2) in float ambientBright;
            layout (location = 3) in vec2 uv;
            out vec3 vertexColor;
            out vec2 TexCoord;
            out vec3 pos;
            uniform mat4 mvp;
            uniform float ambientBrightMult;
            void main()
            {
                gl_Position = mvp * vec4(position, 1.0);

                float ambBright = ambientBrightMult * ambientBright;

                float bright = min(16.0f, blockBright + ambBright);

                vertexColor = vec3(bright/16.0f, bright/16.0f, bright/16.0f);
                TexCoord = uv;
                pos = position;
            }
        )glsl",
        R"glsl(
            #version 330 core
            in vec3 vertexColor;
            in vec2 TexCoord;
            in vec3 pos;
            out vec4 FragColor;
            uniform sampler2D ourTexture;
            uniform vec3 camPos;
            void main()
            {
                vec4 texColor = texture(ourTexture, TexCoord);
                FragColor = texColor * vec4(vertexColor, 1.0);

                vec4 fogColor = vec4(0.7, 0.7, 0.949, 1.0);
                float distance = (distance(pos, camPos)/67.0f)/5.0f;

                if(FragColor.a < 0.4) {
                    discard;
                }

                FragColor = mix(FragColor, fogColor, min(1, max(distance, 0)));
            }
        )glsl",
        "worldShader"
    );
    wireFrameShader = std::make_unique<Shader>(
        R"glsl(
            #version 330 core
            layout (location = 0) in vec3 position;
            uniform mat4 mvp;
            uniform vec3 translation;
            void main()
            {
                gl_Position = mvp * vec4(position + translation, 1.0f);

            }
        )glsl",
        R"glsl(
            #version 330 core
            out vec4 FragColor;
            uniform float displaying;
            void main()
            {
                FragColor = vec4(0.0, 0.0, 0.0, 1.0);
                if(displaying == 0.0f) {
                    discard;
                }
            }
        )glsl",
        "wireFrameShader"
    );
}

void Game::bindMenuGeometry(GLuint vbo, const float *data, size_t dataSize) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(float), data, GL_STATIC_DRAW);

    GLint posAttrib = glGetAttribLocation(menuShader->shaderID, "pos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    GLint texAttrib = glGetAttribLocation(menuShader->shaderID, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    GLint elementIdAttrib = glGetAttribLocation(menuShader->shaderID, "elementid");
    glEnableVertexAttribArray(elementIdAttrib);
    glVertexAttribPointer(elementIdAttrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
}

void Game::bindMenuGeometryNoUpload(GLuint vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLint posAttrib = glGetAttribLocation(menuShader->shaderID, "pos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    GLint texAttrib = glGetAttribLocation(menuShader->shaderID, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    GLint elementIdAttrib = glGetAttribLocation(menuShader->shaderID, "elementid");
    glEnableVertexAttribArray(elementIdAttrib);
    glVertexAttribPointer(elementIdAttrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
}

void Game::bindWorldGeometry(GLuint vbov, GLuint vbouv, const float *vdata, const float *uvdata, size_t vsize, size_t uvsize) {
    GLenum error;
    glBindBuffer(GL_ARRAY_BUFFER, vbov);
    glBufferData(GL_ARRAY_BUFFER, vsize * sizeof(float), vdata, GL_STATIC_DRAW);
    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Bind world geom err (vbov): " << error << std::endl;
    }
    GLint posAttrib = glGetAttribLocation(worldShader->shaderID, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    // Block brightness attribute
    GLint brightnessAttrib = glGetAttribLocation(worldShader->shaderID, "blockBright");
    glEnableVertexAttribArray(brightnessAttrib);
    glVertexAttribPointer(brightnessAttrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Ambient brightness attribute
    GLint ambBrightness = glGetAttribLocation(worldShader->shaderID, "ambientBright");
    glEnableVertexAttribArray(ambBrightness);
    glVertexAttribPointer(ambBrightness, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));


    glBindBuffer(GL_ARRAY_BUFFER, vbouv);
    glBufferData(GL_ARRAY_BUFFER, uvsize * sizeof(float), uvdata, GL_STATIC_DRAW);
    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Bind world geom err (vbouv): " << error << std::endl;
    }
    GLint uvAttrib = glGetAttribLocation(worldShader->shaderID, "uv");
    glEnableVertexAttribArray(uvAttrib);
    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void Game::bindWorldGeometryNoUpload(GLuint vbov, GLuint vbouv) {
    glBindBuffer(GL_ARRAY_BUFFER, vbov);

    GLint posAttrib = glGetAttribLocation(worldShader->shaderID, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE,  5 * sizeof(float), 0);

    // Block brightness attribute
    GLint brightnessAttrib = glGetAttribLocation(worldShader->shaderID, "blockBright");
    glEnableVertexAttribArray(brightnessAttrib);
    glVertexAttribPointer(brightnessAttrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Ambient brightness attribute
    GLint ambBrightness = glGetAttribLocation(worldShader->shaderID, "ambientBright");
    glEnableVertexAttribArray(ambBrightness);
    glVertexAttribPointer(ambBrightness, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));


    glBindBuffer(GL_ARRAY_BUFFER, vbouv);

    GLint uvAttrib = glGetAttribLocation(worldShader->shaderID, "uv");
    glEnableVertexAttribArray(uvAttrib);
    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
}



void Game::bindWireFrameGeometry(GLuint vbov, const float *vdata, size_t vsize) {
    GLenum error;
    glBindBuffer(GL_ARRAY_BUFFER, vbov);
    glBufferData(GL_ARRAY_BUFFER, vsize * sizeof(float), vdata, GL_STATIC_DRAW);
    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Bind wireframe geom err (vbov): " << error << std::endl;
    }
    GLint posAttrib = glGetAttribLocation(wireFrameShader->shaderID, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
}

void Game::bindWireFrameGeometryNoUpload(GLuint vbov) {
    glBindBuffer(GL_ARRAY_BUFFER, vbov);

    GLint posAttrib = glGetAttribLocation(wireFrameShader->shaderID, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE,  3 * sizeof(float), 0);
}



void Game::stepTextureAnim() {

    int width = 288;
    int chans = 4;


    static float timer = 0.0f;
    if(timer < 100) {
        timer += deltaTime*20;
    } else {
        timer = 0;
    }

    //Water
    glm::ivec4 baseColor(0, 45, 100, 210);
    glm::ivec2 coord(0,0);
    int startY = 270, startX = 18, squareSize = 18;
    for(int y = startY; y < startY + squareSize; ++y) {
        for(int x = startX; x < startX + squareSize; ++x) {
            int i = (y * width + x) * chans;
            float addedNoise = std::max(static_cast<float>(voxelWorld.perlin.noise(
                static_cast<float>(coord.x/4.0f), timer, static_cast<float>(coord.y/32.0f))) * 70, -10.0f);

            worldTexturePixels[i]   = std::min(std::max(baseColor.r + static_cast<int>(addedNoise), 0), 254);
            worldTexturePixels[i+1] = std::min(std::max(baseColor.g + static_cast<int>(addedNoise), 0), 254);
            worldTexturePixels[i+2] = std::min(std::max(baseColor.b + static_cast<int>(addedNoise), 0), 254);
            worldTexturePixels[i+3] = std::min(std::max(baseColor.a, 0), 254);
            coord.x++;
        }
        coord.y++;
    }
    glBindTexture(GL_TEXTURE_2D, worldTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, width, GL_RGBA, GL_UNSIGNED_BYTE, worldTexturePixels);
}

void Game::initializeTextures() {
    glGenTextures(1, &menuTexture);
    glBindTexture(GL_TEXTURE_2D, menuTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("assets/gui.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture menutexture" << std::endl;
    }
    stbi_image_free(data);

    glGenTextures(1, &worldTexture);
    glBindTexture(GL_TEXTURE_2D, worldTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    worldTexturePixels = stbi_load("assets/world.png", &width, &height, &nrChannels, 0);
    if (worldTexturePixels)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, worldTexturePixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture worldtexture" << std::endl;
    }

    glGenTextures(1, &menuBackgroundTexture);
    glBindTexture(GL_TEXTURE_2D, menuBackgroundTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/background.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture menubackgroundtexture" << std::endl;
    }
    stbi_image_free(data);

    glGenTextures(1, &splashTexture);
    glBindTexture(GL_TEXTURE_2D, splashTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/splash.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture splashtexture" << std::endl;
    }
    stbi_image_free(data);


    
    glGenTextures(1, &logoTexture);
    glBindTexture(GL_TEXTURE_2D, logoTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/logo.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture logotexture" << std::endl;
    }
    stbi_image_free(data);
}

void Game::drawSky(float top_r, float top_g, float top_b, float top_a,
    float bot_r, float bot_g, float bot_b, float bot_a, float cameraPitch)
{
    glDisable(GL_DEPTH_TEST);

    static GLuint background_vao = 0;
    static GLuint background_shader = 0;

    if (background_vao == 0)
    {
        glGenVertexArrays(1, &background_vao);

        const GLchar* vs_src =
            "#version 450 core\n"
            "out vec2 v_uv;\n"
            "uniform float cpitch;\n"
            "void main()\n"
            " {\n"
            " uint idx = gl_VertexID;\n"
            
            " gl_Position = vec4((idx >> 1), idx & 1, 0.0, 0.5) * 4.0 - 1.0;\n"
            "v_uv = vec2(gl_Position.xy  + 1.0 +(cpitch/100));\n"
            "}";


        const GLchar* fs_src =
            " #version 450 core\n"
            "uniform vec4 top_color;\n"
            "uniform vec4 bot_color;\n"
            "uniform float brightMult;\n"
            "in vec2 v_uv;\n"
            "out vec4 frag_color;\n"

            "void main()\n"
            "{\n"
            "frag_color = bot_color * (1 - v_uv.y) + top_color * v_uv.y;\n"
            "frag_color = frag_color * vec4(brightMult, brightMult, brightMult, 1.0f);\n"
            "}";

        GLuint vs_id, fs_id;
        vs_id = glCreateShader(GL_VERTEX_SHADER);
        fs_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vs_id, 1, &vs_src, NULL);
        glShaderSource(fs_id, 1, &fs_src, NULL);
        glCompileShader(vs_id);

        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(vs_id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vs_id, 512, NULL, infoLog);
            std::cerr << "Vertex shader compilation error: " << infoLog << std::endl;
        }

        glCompileShader(fs_id);


        glGetShaderiv(fs_id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fs_id, 512, NULL, infoLog);
            std::cerr << "Fragment shader compilation error: " << infoLog << std::endl;
        }

        background_shader = glCreateProgram();
        glAttachShader(background_shader, vs_id);
        glAttachShader(background_shader, fs_id);
        glLinkProgram(background_shader);
        glDetachShader(background_shader, fs_id);
        glDetachShader(background_shader, vs_id);
        glDeleteShader(fs_id);
        glDeleteShader(vs_id);
    }

    glBindVertexArray(background_vao);
    glUseProgram(background_shader);
    GLuint top_color_loc = glGetUniformLocation(background_shader, "top_color");
    GLuint bot_color_loc = glGetUniformLocation(background_shader, "bot_color");
    glUniform4f(top_color_loc, top_r, top_g, top_b, top_a);
    glUniform4f(bot_color_loc, bot_r, bot_g, bot_b, bot_a);
    GLuint cpitch_loc = glGetUniformLocation(background_shader, "cpitch");

    glUniform1f(cpitch_loc, cameraPitch);

    GLuint ambBrightLoc = glGetUniformLocation(background_shader, "brightMult");
    glUniform1f(ambBrightLoc, ambientBrightnessMult);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}
