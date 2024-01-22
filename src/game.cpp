#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cmath>


Game::Game() : lastFrame(0), focused(false), camera(nullptr),
collCage([this](BlockCoord b){
    uint32_t blockBitsHere = voxelWorld.blockAt(b);
    uint32_t blockIDHere = blockBitsHere & BlockInfo::BLOCK_ID_BITS;
    if(blockIDHere == 11) {
        if(DoorInfo::getDoorOpenBit(blockBitsHere) == 1) {
            return false;
        } else {
            return true;
        }
    }
    if(blockIDHere == 14) {
        return false;
    }
    return (blockIDHere != 0 && blockIDHere != 2);
}),
user(glm::vec3(0,0,0), glm::vec3(0,0,0)),
grounded(true)
{

    windowWidth = 1280;
    windowHeight = 720;
    camera = new Camera3D(this);
    mouseSensitivity = 0.1;
    averageDeltaTime = 0.0f;

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

            if(!loadRendering && !changingViewDistance) {
                stepMovementAndPhysics();
            }
            

            
            draw();
            

            glfwPollEvents();
            updateTime();
            runPeriodicTick();

            static std::function<float(float, float, float)> gaussian = [](float x, float peak, float radius) {
                float stdDev = radius / 3.0;  // Controls the spread
                float variance = stdDev * stdDev;

                // Gaussian formula
                float b = exp(-pow(x - peak, 2.0) / (2.0 * variance));

                // Normalize the peak to 1
                float peakHeight = exp(-pow(peak - peak, 2.0) / (2.0 * variance));
                return b / peakHeight;
            };

            if(inGame) {
                voxelWorld.runStep(deltaTime);
                timeOfDay = std::fmod(timeOfDay + deltaTime, dayLength);
                ambientBrightnessMult = std::max(0.05f, std::min(1.0f, gaussian(timeOfDay, dayLength/2.0f, dayLength/2.0f) * 1.3f));
                //std::cout << std::to_string(ambientBrightnessMult) << "\n";

                sunsetFactor = gaussian(timeOfDay, dayLength*(3.0f/4.0f), dayLength/16.0f);
                sunriseFactor = gaussian(timeOfDay, dayLength/6.0f, dayLength/16.0f);

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
        if(timer > 2.0f) {
            loopFunc = &normalFunc;
        } else {
            timer += deltaTime;
        }
    };

    loopFunc = &splashFunc;

    std::vector<Setting> sets = {
        Setting{std::string("viewDistance"), std::to_string(viewDistance)}
    };
    settings.loadOrSaveSettings(sets);

    for(Setting &set : sets) {

        if(set.name == std::string("viewDistance")) {
            viewDistance = std::stoi(set.value);
        }

    }



}

void Game::getAverageDelta() {
    static int times = 0;
    static float accum = 0.0f;
    if(times < 100) {
        accum += static_cast<float>(deltaTime);
        times++;
    } else {
        averageDeltaTime = accum / 100;
        times = 0;
        accum = 0.0f;
    }
}

void Game::stepMovementAndPhysics() {


            if(initialTimer > 0.0f) {
                initialTimer -= deltaTime;
                camera->goToPosition(camera->position);
            } else {

            static float currentJumpY = 0.0f;
            float allowableJumpHeight = 1.1f;
            static bool jumpingUp = false;

            static float timeFallingScalar = 1.0f;

               if(std::find(collCage.solid.begin(), collCage.solid.end(), FLOOR) == collCage.solid.end())
                {
                    grounded = false;
                } else {
                    if(leaning) {
                        leanSpot = glm::vec3(std::round(camera->position.x),
                        std::round(camera->position.y),
                        std::round(camera->position.z));
                    }
                }

                glm::vec3 collCageCenter = camera->position + glm::vec3(0, -1.0, 0);
                collCage.update_readings(collCageCenter);

            if(inWater || inClimbable) {

                timeFallingScalar = 1.0f;
                if(!grounded) {
                    camera->velocity += glm::vec3(0.0, -2.0*averageDeltaTime, 0.0);
                    if(camera->downPressed)
                    {
                        camera->velocity += glm::vec3(0.0, -5.0*averageDeltaTime, 0.0);
                    }
                }
                

                if(camera->upPressed)
                {
                    camera->velocity += glm::vec3(0.0, 5.0*averageDeltaTime, 0.0);
                }

                

            } else {




                if(!grounded && !jumpingUp && !leaning) {
                    timeFallingScalar = std::min(timeFallingScalar + averageDeltaTime*5.0f, 3.0f);
                } else {
                    timeFallingScalar = 1.0f;
                }


                

             

                if(!grounded && !jumpingUp && !leaning /*&& jumpTimer <= 0.0f*/)
                {
                    camera->velocity += glm::vec3(0.0, -GRAV*timeFallingScalar*averageDeltaTime, 0.0);
                }

                if(jumpingUp) {
                    if(camera->position.y < currentJumpY + allowableJumpHeight) {
                        camera->velocity += glm::vec3(0.0f, (((currentJumpY+allowableJumpHeight+0.3f) - camera->position.y)*10.0f)*averageDeltaTime, 0.0f);
                    } else {
                        jumpingUp = false;
                    }
                }

                // if(jumpTimer > 0.0f) {
                //     jumpTimer = std::max(0.0, jumpTimer - deltaTime);
                // }

                if(camera->upPressed && grounded && !leaning)
                {
                    //camera->velocity += glm::vec3(0.0, 100.0*deltaTime, 0.0);
                    grounded = false;
                    //jumpTimer = deltaTime*10.0f;
                    currentJumpY = camera->position.y;
                    jumpingUp = true;
                    camera->upPressed = 0;
                }
            }
            glm::vec3 proposedPos;
            if( leaning) {
                proposedPos = camera->proposeSlowPosition();
            } else {
                proposedPos = camera->proposePosition();
            }
            

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
            if(leaning) {
                if(std::sqrt(std::pow(proposedPos.x - leanSpot.x, 2) 
                + std::pow(proposedPos.y - leanSpot.y, 2) 
                + std::pow(proposedPos.z - leanSpot.z, 2)) < 0.7) {
                    camera->goToPosition(proposedPos);
                } else {
                    camera->goToPosition(camera->position);
                }
                    
            } else {
                camera->goToPosition(proposedPos);
            }
            
            }
}


void Game::drawSplashScreen() {

    if(VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);


    float splashImageWidth = 700;



    glm::vec2 splashLowerLeft(-splashImageWidth/windowWidth, -splashImageWidth/windowHeight);
    float relHeight = splashImageWidth/(windowHeight/2);
    float relWidth = splashImageWidth/(windowWidth/2);


    std::vector<float> splashDisplayData = {
        splashLowerLeft.x, splashLowerLeft.y,                    0.0f, 1.0f,   -997.0f,
        splashLowerLeft.x, splashLowerLeft.y+relHeight,          0.0f, 0.0f,   -997.0f,
        splashLowerLeft.x+relWidth, splashLowerLeft.y+relHeight, 1.0f, 0.0f,   -997.0f,

        splashLowerLeft.x+relWidth, splashLowerLeft.y+relHeight, 1.0f, 0.0f,   -997.0f,
        splashLowerLeft.x+relWidth, splashLowerLeft.y,           1.0f, 1.0f,   -997.0f,
        splashLowerLeft.x, splashLowerLeft.y,                    0.0f, 1.0f,   -997.0f,

   
    };


    std::vector<float> splashDisplayData2 = {
        splashLowerLeft.x, splashLowerLeft.y,                    0.0f, 1.0f,   -998.0f,
        splashLowerLeft.x, splashLowerLeft.y+relHeight,          0.0f, 0.0f,   -998.0f,
        splashLowerLeft.x+relWidth, splashLowerLeft.y+relHeight, 1.0f, 0.0f,   -998.0f,

        splashLowerLeft.x+relWidth, splashLowerLeft.y+relHeight, 1.0f, 0.0f,   -998.0f,
        splashLowerLeft.x+relWidth, splashLowerLeft.y,           1.0f, 1.0f,   -998.0f,
        splashLowerLeft.x, splashLowerLeft.y,                    0.0f, 1.0f,   -998.0f
    };



    glUseProgram(menuShader->shaderID);

    GLuint timeLocation = glGetUniformLocation(menuShader->shaderID, "time");
    glUniform1f(timeLocation, static_cast<float>(glfwGetTime()));

   
    glBindTexture(GL_TEXTURE_2D, splashTexture);


    static GLuint vbo = 0;
    if(vbo == 0) {
        glGenBuffers(1, &vbo);
    }

    static GLuint vbo2 = 0;
    if(vbo2 == 0) {
        glGenBuffers(1, &vbo2);
    }

        bindMenuGeometry(vbo, 
        splashDisplayData.data(),
        splashDisplayData.size());


    glDrawArrays(GL_TRIANGLES, 0, splashDisplayData.size()/5);

    glBindTexture(GL_TEXTURE_2D, splashTexture2);
    bindMenuGeometry(vbo2, 
        splashDisplayData2.data(),
        splashDisplayData2.size());

    glDrawArrays(GL_TRIANGLES, 0, splashDisplayData2.size()/5);
    

    glfwSwapBuffers(window);
}

float similarity(glm::vec3 dir1, glm::vec3 dir2) {
    return (glm::dot(glm::normalize(dir1), glm::normalize(dir2)) + 1.0) * 0.5;
}

void Game::draw() {
    #ifdef DEV
        if(voxelWorld.numberOfSamples >= 100) {
            float average = voxelWorld.timeChunkMeshing / voxelWorld.numberOfSamples;
            std::cout << "Average chunk time: " << average << "\n";
            voxelWorld.timeChunkMeshing = 0.0f;
            voxelWorld.numberOfSamples = 0;
        }
    #endif
    
    if(VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenVertexArrays(1, &VAO2);
    }


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClearColor(0.639, 0.71, 1.0, 0.5);

    glm::vec4 skyBottom(1.0f, 1.0f, 1.0f, 1.0f);

    skyBottom = glm::mix(skyBottom, glm::vec4(1.0f, 0.651f, 0.0f, 1.0f), (similarity(camera->direction, glm::vec3(0, 0, -1)) * 0.7f) * sunsetFactor );
    skyBottom = glm::mix(skyBottom, glm::vec4(1.0f, 0.651f, 0.0f, 1.0f), (similarity(camera->direction, glm::vec3(0, 0, 1)) * 0.7f) * sunriseFactor );
    glBindVertexArray(VAO);
    if(underWaterView == 1.0f) {
        drawSky(0.2f, 0.2f, 1.0f, 1.0f,    0.2f, 0.2f, 1.0f, 1.0f,   camera->pitch);
    } else {
        drawSky(skyColor.r, skyColor.g, skyColor.b, 1.0f,    skyBottom.r, skyBottom.g, skyBottom.b, skyBottom.a, camera->pitch);
    }
    
    
    drawParticles();



    glBindVertexArray(VAO);
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


            std::vector<float> logoDisplayData;

            float oneOverNine = 1.0f/9.0f;

            for(int i = 0; i < 9; ++i) {
                logoDisplayData.insert(logoDisplayData.end(), {
                    logoLowerLeft.x + i*(relWidth/9),               logoLowerLeft.y + shiftUp,                    0.0f + i*oneOverNine,      1.0f,   -67.0f - (i*1.0f),
                    logoLowerLeft.x + i*(relWidth/9),               logoLowerLeft.y+relHeight+ shiftUp,          0.0f + i*oneOverNine,       0.0f,   -67.0f - (i*1.0f),
                    logoLowerLeft.x +(relWidth/9) + i*(relWidth/9), logoLowerLeft.y+relHeight+ shiftUp,         oneOverNine + i*oneOverNine, 0.0f,   -67.0f - (i*1.0f),

                    logoLowerLeft.x +(relWidth/9) + i*(relWidth/9), logoLowerLeft.y+relHeight+ shiftUp,          oneOverNine + i*oneOverNine, 0.0f,   -67.0f - (i*1.0f),
                    logoLowerLeft.x +(relWidth/9) + i*(relWidth/9), logoLowerLeft.y+ shiftUp,                    oneOverNine + i*oneOverNine, 1.0f,   -67.0f - (i*1.0f),
                    logoLowerLeft.x + i*(relWidth/9),               logoLowerLeft.y+ shiftUp,                    0.0f + i*oneOverNine,        1.0f,   -67.0f - (i*1.0f)
                });
            }

            GLuint timeLocation = glGetUniformLocation(menuShader->shaderID, "time");
            glUniform1f(timeLocation, static_cast<float>(std::fmod(glfwGetTime()-2.0, 7.0f)));

        
            glBindTexture(GL_TEXTURE_2D, logoTexture);


            static GLuint vbo = 0;
            if(vbo == 0) {
                glGenBuffers(1, &vbo);
            }

                bindMenuGeometry(vbo, 
                logoDisplayData.data(),
                logoDisplayData.size());


            glDrawArrays(GL_TRIANGLES, 0, logoDisplayData.size()/5);


            const char* message = "Version 0.0.3b";

            std::vector<float> displayData;


            float letHeight = (32.0f/windowHeight);
            float letWidth = (32.0f/windowWidth);
            float lettersCount = std::strlen(message);
            float totletwid = letWidth * lettersCount;
            glm::vec2 letterStart(-totletwid/2, -letHeight/2 + 0.2f);

            GlyphFace glyph;

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

            static GLuint vbo2 = 0;
            if(vbo2 == 0) {
                glGenBuffers(1, &vbo2);
            }

                bindMenuGeometry(vbo2, 
                displayData.data(),
                displayData.size());

            glBindTexture(GL_TEXTURE_2D, menuTexture);
            glDrawArrays(GL_TRIANGLES, 0, displayData.size()/5);


        }

    }

    if(!inGame) {
        drawBackgroundImage();
    }

    if(inGame) {
        //std::cout << loadRendering << "\n";

        if(!loadRendering && !noHud)
        {
            glBindVertexArray(VAO);
            drawSelectedBlock();
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
        }
        if(!changingViewDistance) {
            stepChunkDraw();
        }


        
        glBindVertexArray(VAO);
        if(!noHud) {
            updateAndDrawSelectCube();
            drawBlockOverlay();
        }
            
    }


    glfwSwapBuffers(window);
}

void Game::drawBackgroundImage() {
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

    static float numMustLoad = (viewDistance*2)*(viewDistance);

    GLuint mvp_loc = glGetUniformLocation(worldShader->shaderID, "mvp");
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(camera->mvp));

    GLuint cam_pos_loc = glGetUniformLocation(worldShader->shaderID, "camPos");
    glUniform3f(cam_pos_loc, camera->position.x, camera->position.y, camera->position.z);

    GLuint cam_dir_loc = glGetUniformLocation(worldShader->shaderID, "camDir");
    glUniform3f(cam_dir_loc, camera->direction.x, camera->direction.y, camera->direction.z);

    GLuint viewDistLoc = glGetUniformLocation(worldShader->shaderID, "viewDistance");
    glUniform1f(viewDistLoc, viewDistance);

    BlockCoord headCoord(
        std::round(camera->position.x),
        std::round(camera->position.y),
        std::round(camera->position.z)
    );
    if(voxelWorld.blockAt(headCoord) == 2) {
        underWaterView = 1.0f;
    } else {
        underWaterView = 0.0f;
    }

    BlockCoord feetCoord(
        std::round(camera->position.x),
        std::ceil(camera->position.y-1.25f),
        std::round(camera->position.z)
    );

    BlockCoord feetCoord2(
        std::round(camera->position.x),
        std::round(camera->position.y-1.0f),
        std::round(camera->position.z)
    );

    static bool previousInWater  = false;

    uint32_t blockUsersIn = voxelWorld.blockAt(feetCoord) & BlockInfo::BLOCK_ID_BITS;
    uint32_t blockUsersInLowered = voxelWorld.blockAt(feetCoord2) & BlockInfo::BLOCK_ID_BITS;

    if(blockUsersIn == 2) {
        inWater = true;
        if(previousInWater != inWater) {
            previousInWater = inWater;
            splashyParticles(feetCoord2, 15);
        }
    } 

    if(blockUsersIn == 14) {
        inClimbable = true;
    }

    if(blockUsersInLowered != 14) {
        inClimbable = false;
    }

    if(blockUsersInLowered != 2)
    {
        inWater = false;
        previousInWater = false;
    }


    GLuint ambBrightMultLoc = glGetUniformLocation(worldShader->shaderID, "ambientBrightMult");

    glUniform1f(ambBrightMultLoc, ambientBrightnessMult);

    GLuint ssloc = glGetUniformLocation(worldShader->shaderID, "sunset");

    glUniform1f(ssloc, sunsetFactor);

    GLuint srloc = glGetUniformLocation(worldShader->shaderID, "sunrise");

    glUniform1f(srloc, sunriseFactor);

    GLuint uwLoc = glGetUniformLocation(worldShader->shaderID, "underWater");
    glUniform1f(uwLoc, underWaterView);

    voxelWorld.cameraPosition = camera->position;
    voxelWorld.cameraDirection = camera->direction;

    int poppedIndex = 0;
    if(voxelWorld.highPriorityGeometryStoreQueue.pop(poppedIndex)) {

            GeometryStore &geometryStore = voxelWorld.geometryStorePool[poppedIndex];
            if(geometryStore.myLock.try_lock()) {
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
                geometryStore.myLock.unlock();
                if(loadRendering) {

                    initialChunksRendered += 1;
                    //std::cout << initialChunksRendered << "\n";
                    if(initialChunksRendered >= numMustLoad) {
                        //std::cout << "Done! Rendered " << numMustLoad << " chunks.\n";
                        loadRendering = false;
                    }
                }
            } else {
                while(!voxelWorld.highPriorityGeometryStoreQueue.push(poppedIndex)) {

                }
            }

    } else {
        if(voxelWorld.geometryStoreQueue.pop(poppedIndex)) {
            GeometryStore &geometryStore = voxelWorld.geometryStorePool[poppedIndex];
            if(geometryStore.myLock.try_lock()) {
                if(!registry.all_of<MeshComponent>(geometryStore.me)) {
                    MeshComponent m;
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
                geometryStore.myLock.unlock();
                if(loadRendering) {

                    initialChunksRendered += 1;
                    //std::cout << initialChunksRendered << "\n";
                    if(initialChunksRendered >= numMustLoad) {
                        //std::cout << "Done! Rendered " << numMustLoad << " chunks.\n";
                        loadRendering = false;
                    }
                }
            } else {
                while(!voxelWorld.geometryStoreQueue.push(poppedIndex)) {

                }
            }
        }
    }

    if(loadRendering) {
        float progress = (initialChunksRendered) / (numMustLoad);
        displayLoadScreen("Loading world", progress, true);
    } else {
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
            voxelWorld.nonUserDataMap.clear();
            voxelWorld.hasHadInitialLightPass.clear();
            int throwaway = 0;
            while(voxelWorld.geometryStoreQueue.pop(throwaway)) {

            }
            while(voxelWorld.highPriorityGeometryStoreQueue.pop(throwaway)) {

            }

            BlockChunk *throwaway2 = 0;
            while(voxelWorld.deferredChunkQueue.pop(throwaway)) {

            }
            
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
        GUIButton(0.0f, -0.1f, "Settings", 0.0f, 3.0f, [this](){
            goToSettingsMenu();
        }),
        GUIButton(0.0f, -0.2f, "Back to game", 0.0f, 2.0f, [this](){
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


void Game::goToSettingsMenu() {

    static std::string rendDist = (std::string("Render Distance: ") + std::to_string(viewDistance));

    static std::function<void()> updateNum = [this](){

        rendDist = std::string("Render Distance: ") + std::to_string(viewDistance);
        currentGuiButtons->at(0).label = rendDist.c_str(); 
        for(GUIButton &button : *currentGuiButtons) {
            button.rebuildDisplayData();
            button.uploaded = false;
        }
    };  
    static std::vector<GUIButton> buttons = {
        GUIButton(0.0f, 0.0f, (std::string("Render Distance: ") + std::to_string(viewDistance)).c_str(), 0.0f, -1.0f, [this](){
            
        }),
        GUIButton(-0.2f, -0.1f, "<", 0.0f, 3.0f, [this](){
            changeViewDistance(std::max(viewDistance-1, 2));
            updateNum();
            
        }),
        GUIButton(0.2f, -0.1f, ">", 0.0f, 4.0f, [this](){
            changeViewDistance(std::min(viewDistance+1, 24));
            updateNum();
        }),
        GUIButton(0.0f, -0.2f, "Back", 0.0f, 5.0f, [this](){
            displayEscapeMenu();
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
        GUIButton(0.0f, 0.2f, "World 1", 0.55f, 1.0f, [this](){
            goToSingleplayerWorld("world1");
        }),
        GUIButton(0.0f, 0.1f, "World 2", 0.55f, 2.0f, [this](){
            goToSingleplayerWorld("world2");
        }),
        GUIButton(0.0f, 0.0f, "World 3", 0.55f, 3.0f, [this](){
            goToSingleplayerWorld("world3");
        }),
        GUIButton(0.0f, -0.1f, "World 4", 0.55f, 4.0f, [this](){
            goToSingleplayerWorld("world4");
        }),
        GUIButton(0.0f, -0.2f, "World 5", 0.55f, 5.0f, [this](){
            goToSingleplayerWorld("world5");
        }),
        GUIButton(0.0f, -0.4f, "Back to main menu", 0.0f, 6.0f, [this](){
            this->goToMainMenu();
        }),
    };

    static std::vector<std::string> labels = {
        std::string("World 1"),
        std::string("World 2"),
        std::string("World 3"),
        std::string("World 4"),
        std::string("World 5")
    };

    for(int i = 0; i < 5; i++) {
        std::string thisPath = "saves/world";
        thisPath += std::to_string(i+1);
        if(voxelWorld.checkVersionOfSave(thisPath.c_str()) == 1) {
            labels[i] = (std::string("World ") + std::to_string(i+1)) + " (Classic)";
            buttons[i].label = labels[i].c_str();
        } else {
            labels[i] = (std::string("World ") + std::to_string(i+1));
            buttons[i].label = labels[i].c_str();
        }
    }

    for(int i = 1; i < 6; ++i) {
        std::string thisPath = "saves/world";
        thisPath += std::to_string(i);
        if(voxelWorld.saveExists(thisPath.c_str())) {
            auto buttonIt = std::find_if(buttons.begin(), buttons.end(), [i](GUIButton& button){
                return button.elementID == i + 6;
            });
            if(buttonIt == buttons.end()) {
                buttons.push_back(
                    GUIButton(0.7f, 0.2f + (-0.1f * (i-1)), "X", 0.0f, static_cast<float>(i + 6), [this, i](){
                        goToConfirmDeleteWorld(i);
                    })
                );
            }
        } else {
            auto buttonIt = std::find_if(buttons.begin(), buttons.end(), [i](GUIButton& button){
                return button.elementID == i + 6;
            });
            if(buttonIt != buttons.end()) {
                buttons.erase(buttonIt);
            }
        }
    } 
    for(GUIButton &button : buttons) {
        button.rebuildDisplayData();
        button.uploaded = false;
    }
    currentGuiButtons = &buttons;
}

void Game::goToConfirmDeleteWorld(int num) {
    static std::function<void()> deleteFunc;

    deleteFunc = [this, num]() {
        voxelWorld.deleteFolder(std::string("saves/world") + std::to_string(num));
        goToSingleplayerWorldsMenu();
        //std::cout << "Would delete " << std::string("saves/world") + std::to_string(num) << "\n";
    };

    static std::vector<GUIButton> buttons = {
        GUIButton(0.0f, 0.2f, "Are you sure you want to delete ", 0.55f, -97.0f, [](){}),
        GUIButton(0.0f, 0.1f, "There is NO UNDO for this action.", 0.55f, -97.0f, [](){}),
        GUIButton(0.0f, -0.1f, "Delete this world", 0.55f, 1.0f, [this](){
            
        }),
        GUIButton(0.0f, -0.2f, "Cancel", 0.55f, 2.0f, [this](){
            goToSingleplayerWorldsMenu();
        }),
    };

    buttons.at(2).myFunction = deleteFunc;

    static std::vector<std::string> labels = {
        std::string("Are you sure you want to delete ")
    };

    labels.at(0) = std::string("Are you sure you want to delete World ") + std::to_string(num) + " forever?";
    buttons.at(0).label = labels.at(0).c_str();

    for(GUIButton &button : buttons) {
        button.rebuildDisplayData();
        button.uploaded = false;
    }
    currentGuiButtons = &buttons;
}

void Game::loadOrCreateSaveGame(const char* path) {
    timeOfDay = dayLength/4.0f;
    if(voxelWorld.saveExists(path)) {
        voxelWorld.loadWorldFromFile(path);
    } else {
        voxelWorld.worldGenVersion = 2;
        voxelWorld.waterLevel = 40;
        voxelWorld.currentNoiseFunction = &(voxelWorld.worldGenFunctions.at(2));
        voxelWorld.seed = time(NULL);
        voxelWorld.getOffsetFromSeed();
        camera->position = glm::vec3(0,100,0);
        camera->velocity = glm::vec3(0,0,0);
        //camera->updatePosition();
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
                if(lineIndex == 3) {
                    timeOfDay = std::stof(word);
                }
                localIndex++;
            }
            lineIndex++;
        }
    playerFile.close();
    camera->yaw = loadedYaw;
    camera->pitch = loadedPitch;
    camera->position = loadedPosition;
    camera->direction = loadedDirection;
    camera->firstMouse = true;
    camera->right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camera->direction));
    camera->up = glm::cross(camera->direction, camera->right);
    camera->updatePosition();
    } else {
        std::cerr << "Couldn't open player file when loading. \n";
    }
    
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

        playerFile << timeOfDay << "\n";
    } else {
        std::cerr << "Couldn't open player file when saving. \n";
    }
    playerFile.close();
}

void Game::changeViewDistance(int newValue) {

    voxelWorld.runChunkThread = false;
    changingViewDistance = true;
    // if(voxelWorld.chunkUpdateThread.joinable()) {
    //     voxelWorld.chunkUpdateThread.join();
    // }

    while(voxelWorld.stillRunningThread) {
        //wait
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }


    

    int throwaway = 0;
    while(voxelWorld.geometryStoreQueue.pop(throwaway)) {

    }
    while(voxelWorld.highPriorityGeometryStoreQueue.pop(throwaway)) {

    }

    BlockChunk *throwaway2 = 0;
    while(voxelWorld.deferredChunkQueue.pop(throwaway)) {

    }

    voxelWorld.takenCareOfChunkSpots.clear();
    voxelWorld.geometryStorePool.clear();
    voxelWorld.chunks.clear();
    voxelWorld.hasHadInitialLightPass.clear();

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

    voxelWorld.populateChunksAndGeometryStores(registry, viewDistance);


    viewDistance = newValue;

        voxelWorld.runChunkThread = true;

        voxelWorld.chunkUpdateThread = std::thread([this](){
            voxelWorld.chunkUpdateThreadFunction(viewDistance);
        });
        voxelWorld.chunkUpdateThread.detach();
    changingViewDistance = false;

    saveSettings();
}

void Game::drawSelectedBlock() {
    float splashImageWidth = 200;

    static unsigned int lastSelectedBlockID = 0;
    static int lastWindowWidth = 0;
    static int lastWindowHeight = 0;

    float horizontalScale = 0.5f;
    float verticalScale = 0.3f;

    float xOffset = static_cast<float>(windowWidth - splashImageWidth) / -windowWidth;
    float yOffset = static_cast<float>(windowHeight - splashImageWidth) / -windowHeight;


    glm::vec2 splashLowerLeft((-splashImageWidth/windowWidth)* horizontalScale + xOffset, (-splashImageWidth/windowHeight) * verticalScale + yOffset);
    float relHeight = (splashImageWidth/(windowHeight/2)) * verticalScale;
    float relWidth = (splashImageWidth/(windowWidth/2)) * horizontalScale;

    static std::vector<float> splashDisplayData;

    if(lastSelectedBlockID != selectedBlockID || windowWidth != lastWindowWidth || windowHeight != lastWindowHeight) {
        lastSelectedBlockID = selectedBlockID;
        lastWindowWidth = windowWidth;
        lastWindowHeight = windowHeight;
        TextureFace top = BlockInfo::texs[selectedBlockID][0];
        TextureFace sides = BlockInfo::texs[selectedBlockID][1];
        splashDisplayData = {
            splashLowerLeft.x+(relWidth/4), splashLowerLeft.y+relHeight*(0.75f),   top.tl.x, top.tl.y,   -1.0f,
            splashLowerLeft.x+(relWidth/2), splashLowerLeft.y+relHeight,   top.tr.x, top.tr.y,   -1.0f,
            splashLowerLeft.x+(relWidth*0.75f), splashLowerLeft.y+relHeight*(0.75f),   top.br.x, top.br.y,   -1.0f,

            splashLowerLeft.x+(relWidth*0.75f), splashLowerLeft.y+relHeight*(0.75f),   top.br.x, top.br.y,   -1.0f,
            splashLowerLeft.x+(relWidth*0.5f), splashLowerLeft.y+relHeight*(0.5f),   top.bl.x, top.bl.y,   -1.0f,
            splashLowerLeft.x+(relWidth/4), splashLowerLeft.y+relHeight*(0.75f),   top.tl.x, top.tl.y,   -1.0f,


            splashLowerLeft.x+(relWidth/4), splashLowerLeft.y+relHeight/4,   sides.bl.x, sides.bl.y,   -99.0f,
            splashLowerLeft.x+(relWidth/4), splashLowerLeft.y+relHeight*0.75f,   sides.tl.x, sides.tl.y,   -99.0f,
            splashLowerLeft.x+(relWidth/2), splashLowerLeft.y+relHeight/2,   sides.tr.x, sides.tr.y,   -99.0f,

            splashLowerLeft.x+(relWidth/2), splashLowerLeft.y+relHeight/2,   sides.tr.x, sides.tr.y,   -99.0f,
            splashLowerLeft.x+(relWidth/2), splashLowerLeft.y,               sides.br.x, sides.br.y,   -99.0f,
            splashLowerLeft.x+(relWidth/4), splashLowerLeft.y+relHeight/4,   sides.bl.x, sides.bl.y,   -99.0f,

            splashLowerLeft.x+(relWidth/2), splashLowerLeft.y,               sides.bl.x, sides.bl.y,   -98.0f,
            splashLowerLeft.x+(relWidth/2), splashLowerLeft.y+(relHeight/2),  sides.tl.x, sides.tl.y,   -98.0f,
            splashLowerLeft.x+(relWidth*0.75f), splashLowerLeft.y+(relHeight*0.75f),  sides.tr.x, sides.tr.y,   -98.0f,

            splashLowerLeft.x+(relWidth*0.75f), splashLowerLeft.y+(relHeight*0.75f),  sides.tr.x, sides.tr.y,   -98.0f,
            splashLowerLeft.x+(relWidth*0.75f), splashLowerLeft.y+(relHeight*0.25f),  sides.br.x, sides.br.y,   -98.0f,
            splashLowerLeft.x+(relWidth/2), splashLowerLeft.y,               sides.bl.x, sides.bl.y,   -98.0f,
        };
    }



    glUseProgram(menuShader->shaderID);

   
    glBindTexture(GL_TEXTURE_2D, worldTexture);


    static GLuint vbo = 0;
    if(vbo == 0) {
        glGenBuffers(1, &vbo);
    }

        bindMenuGeometry(vbo, 
        splashDisplayData.data(),
        splashDisplayData.size());


    glDrawArrays(GL_TRIANGLES, 0, splashDisplayData.size()/5);

}



void Game::goToSingleplayerWorld(const char *worldname) {

    initialTimer = 1.0f;
    voxelWorld.initialLoadProgress = 0;
    loadRendering = true;

    voxelWorld.populateChunksAndGeometryStores(registry, viewDistance);



    currentGuiButtons = nullptr;

    currentSingleplayerWorldPath = std::string("saves/") + std::string(worldname);

    loadOrCreateSaveGame(currentSingleplayerWorldPath.c_str());


    voxelWorld.runChunkThread = true;
    voxelWorld.chunkUpdateThread = std::thread([this](){
        voxelWorld.chunkUpdateThreadFunction(viewDistance);
        });
    voxelWorld.chunkUpdateThread.detach();







    camera->setFocused(true);

    inGame = true;
}

void Game::displayLoadScreen(const char* message, float progress, bool inMainLoop) {

if(!inMainLoop) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.639, 0.71, 1.0, 0.5);
}

    glBindVertexArray(VAO);



        glUseProgram(menuShader->shaderID);
        glBindTexture(GL_TEXTURE_2D, menuTexture);

        static GLuint lsvbo = 0;
            glDeleteBuffers(1, &lsvbo);
            glGenBuffers(1, &lsvbo);


        float width = (500.0f/windowWidth);
        float height = (40.0f/windowHeight);

        glm::vec2 leftStart(-width/2.0f, -height/2.0f);

        TextureFace blank(0,1);
        TextureFace full(1,1);

        std::vector<float> displayData = {
            leftStart.x,                leftStart.y,        full.bl.x,  full.bl.y,  -1.0f,
            leftStart.x,                leftStart.y+height, full.tl.x,  full.tl.y,  -1.0f,
            leftStart.x+width*progress, leftStart.y+height, full.tr.x,  full.tr.y,  -1.0f,

            leftStart.x+width*progress, leftStart.y+height, full.tr.x,  full.tr.y,  -1.0f,
            leftStart.x+width*progress, leftStart.y,        full.br.x,  full.br.y,  -1.0f,
            leftStart.x,                leftStart.y,        full.bl.x,  full.bl.y,  -1.0f,

            leftStart.x,                leftStart.y,        blank.bl.x, blank.bl.y, -1.0f,
            leftStart.x,                leftStart.y+height, blank.tl.x, blank.tl.y, -1.0f,
            leftStart.x+width,          leftStart.y+height, blank.tr.x, blank.tr.y, -1.0f,

            leftStart.x+width,          leftStart.y+height, blank.tr.x, blank.tr.y, -1.0f,
            leftStart.x+width,          leftStart.y,        blank.br.x, blank.br.y, -1.0f,
            leftStart.x,                leftStart.y,        blank.bl.x, blank.bl.y, -1.0f
        };

        float letHeight = (32.0f/windowHeight);
        float letWidth = (32.0f/windowWidth);
        float lettersCount = std::strlen(message);
        float totletwid = letWidth * lettersCount;
        glm::vec2 letterStart(-totletwid/2, -letHeight/2 + 0.2f);

        GlyphFace glyph;

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
        
        bindMenuGeometry(lsvbo, displayData.data(), displayData.size());

        glDrawArrays(GL_TRIANGLES, 0, displayData.size() / 5);

        drawBackgroundImage();

    glBindVertexArray(0);
    if(!inMainLoop) {
        glfwSwapBuffers(window);
    }
}

void Game::updateTime() {
    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    getAverageDelta();
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
            {
                blockOverlayShowing = true;
                blockBreakingTimer = 0.0f;
            }
                
            if(button == GLFW_MOUSE_BUTTON_RIGHT)
                castPlaceRay();
        }

        clickedOnElement = mousedOverElement;
    } else {

        if(inGame && camera->focused) {
            if(button == GLFW_MOUSE_BUTTON_LEFT)
                blockOverlayShowing = false;
        }

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
            uint32_t block = voxelWorld.blockAt(coord);
            return block != 0 && block != 2;
        },
        true
    );
    if(rayResult.hit) {
        uint32_t blockBitsHere = voxelWorld.blockAt(rayResult.blockHit);
        uint32_t blockIDHere = blockBitsHere & BlockInfo::BLOCK_ID_BITS;
        if(blockIDHere == 11) {
            int top = DoorInfo::getDoorTopBit(blockBitsHere);
            BlockCoord otherHalf;
            if(top == 1) {
                otherHalf = rayResult.blockHit + BlockCoord(0, -1, 0);
            } else {
                otherHalf = rayResult.blockHit + BlockCoord(0, 1, 0);
            }

            voxelWorld.userDataMap.at(rayResult.chunksToRebuild.front()).insert_or_assign(otherHalf, 0);    
            voxelWorld.userDataMap.at(rayResult.chunksToRebuild.front()).insert_or_assign(rayResult.blockHit, 0);

            auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(rayResult.chunksToRebuild.front());
                if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                    
                    BlockChunk *chunk = chunkIt->second;

                    while(!voxelWorld.deferredChunkQueue.push(chunk)) {

                    }

                }
        }else
        if(blockIDHere == 12) {
            if(rayResult.chunksToRebuild.size() > 0) {
            if(voxelWorld.userDataMap.find(rayResult.chunksToRebuild.front()) == voxelWorld.userDataMap.end()) {
                voxelWorld.userDataMap.insert_or_assign(rayResult.chunksToRebuild.front(), 
                std::unordered_map<BlockCoord, unsigned int, IntTupHash>());
            }
                   blockBreakParticles(rayResult.blockHit, 25);
            voxelWorld.userDataMap.at(rayResult.chunksToRebuild.front()).insert_or_assign(rayResult.blockHit, 0);

                

                for(ChunkCoord& ccoord : rayResult.chunksToRebuild) {
                    auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(ccoord);
                    if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                        //std::cout << "it's here" << "\n";
                      //  std::cout << "fucking index:" << chunkIt->second.geometryStorePoolIndex << "\n";
                       // voxelWorld.rebuildChunk(chunkIt->second, chunkIt->second.position, true);


                       BlockChunk *chunk = chunkIt->second;


                       while(!voxelWorld.lightUpdateQueue.push(chunk)) {

                       }


                    }
                }
        }
        } else
        if(rayResult.chunksToRebuild.size() > 0) {
            if(voxelWorld.userDataMap.find(rayResult.chunksToRebuild.front()) == voxelWorld.userDataMap.end()) {
                voxelWorld.userDataMap.insert_or_assign(rayResult.chunksToRebuild.front(), 
                std::unordered_map<BlockCoord, unsigned int, IntTupHash>());
            }
                   blockBreakParticles(rayResult.blockHit, 25);
            voxelWorld.userDataMap.at(rayResult.chunksToRebuild.front()).insert_or_assign(rayResult.blockHit, 0);

                std::set<BlockChunk *> implicated;
                for(BlockCoord& neigh : BlockInfo::neighbors) {
                    auto segIt = voxelWorld.lightMap.find(rayResult.blockHit + neigh);
                    if(segIt != voxelWorld.lightMap.end()) {
                        
                        for(LightRay& ray : segIt->second.rays) {
                            ChunkCoord chunkOfOrigin(
                                std::floor(static_cast<float>(ray.origin.x)/voxelWorld.chunkWidth),
                                std::floor(static_cast<float>(ray.origin.z)/voxelWorld.chunkWidth)
                            );
                            auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(chunkOfOrigin);
                            if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()){
                                implicated.insert(chunkIt->second);
                            }
                                
                        }
                        
                    }
                }
                for(BlockChunk * pointer : implicated) {
                    while(!voxelWorld.lightUpdateQueue.push(pointer)) {

                    }
                }
                




                for(ChunkCoord& ccoord : rayResult.chunksToRebuild) {
                    auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(ccoord);
                    if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                        //std::cout << "it's here" << "\n";
                      //  std::cout << "fucking index:" << chunkIt->second.geometryStorePoolIndex << "\n";
                       // voxelWorld.rebuildChunk(chunkIt->second, chunkIt->second.position, true);


                       BlockChunk *chunk = chunkIt->second;


                       while(!voxelWorld.deferredChunkQueue.push(chunk)) {

                       }


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
            uint32_t block = voxelWorld.blockAt(coord);
            return block != 0 && block != 2;
        },
        false
    );
    if(rayResult.hit) {

        uint32_t blockBitsHere = voxelWorld.blockAt(rayResult.blockHit);
        uint32_t blockIDHere = blockBitsHere & BlockInfo::BLOCK_ID_BITS;
        if(blockIDHere == 11) {
            int top = DoorInfo::getDoorTopBit(blockBitsHere);
            BlockCoord otherHalf;
            if(top == 1) {
                otherHalf = rayResult.blockHit + BlockCoord(0, -1, 0);
            } else {
                otherHalf = rayResult.blockHit + BlockCoord(0, 1, 0);
            }
            uint32_t otherHalfBits = voxelWorld.blockAt(otherHalf);

            DoorInfo::toggleDoorOpenBit(blockBitsHere);
            DoorInfo::toggleDoorOpenBit(otherHalfBits);

            voxelWorld.userDataMap.at(rayResult.chunksToRebuild.front()).insert_or_assign(otherHalf, otherHalfBits);    
            voxelWorld.userDataMap.at(rayResult.chunksToRebuild.front()).insert_or_assign(rayResult.blockHit, blockBitsHere);

            auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(rayResult.chunksToRebuild.front());
                if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                    
                    BlockChunk *chunk = chunkIt->second;

                    while(!voxelWorld.deferredChunkQueue.push(chunk)) {

                    }

                }
        }else

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
            //std::cout << "Block hit: " << rayResult.blockHit.x << " " << rayResult.blockHit.y << " " << rayResult.blockHit.z << "\n";
            //std::cout << "Place normal: " << hitNormal.x << " " << hitNormal.y << " " << hitNormal.z << "\n";



            BlockCoord placePoint(rayResult.blockHit.x+hitNormal.x, rayResult.blockHit.y+hitNormal.y, rayResult.blockHit.z+hitNormal.z);
            
            ChunkCoord chunkToReb(
                static_cast<int>(std::floor(static_cast<float>(placePoint.x)/voxelWorld.chunkWidth)),
                static_cast<int>(std::floor(static_cast<float>(placePoint.z)/voxelWorld.chunkWidth)));
            if(voxelWorld.userDataMap.find(chunkToReb) == voxelWorld.userDataMap.end()) {
                voxelWorld.userDataMap.insert_or_assign(chunkToReb, std::unordered_map<BlockCoord, unsigned int, IntTupHash>());
            }
            if(selectedBlockID == 14) {
                static std::vector<BlockCoord> neighborAxes = {
                    BlockCoord(1,0,0),
                    BlockCoord(0,0,1),
                    BlockCoord(1,0,0),
                    BlockCoord(0,0,1),
                };

                
                    uint32_t ladderID = 14;

                    float diffX = camera->position.x - placePoint.x;
                    float diffZ = camera->position.z - placePoint.z;

                    int direction = 0;

                    if (std::abs(diffX) > std::abs(diffZ)) {
                        // The player is primarily aligned with the X-axis
                        direction = diffX > 0 ? /*Plus X*/1 : /*Minus X*/3;
                    } else {
                        // The player is primarily aligned with the Z-axis
                        direction = diffZ > 0 ? /*Plus Z*/2 : /*Minus Z*/0;
                    }

                    BlockInfo::setDirectionBits(ladderID, direction);



                    voxelWorld.userDataMap.at(chunkToReb).insert_or_assign(placePoint, ladderID);

                    auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(chunkToReb);
                    if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                        
                        BlockChunk *chunk = chunkIt->second;


                        while(!voxelWorld.deferredChunkQueue.push(chunk)) {

                        }

                    }
                
            } else
            if(selectedBlockID == 13) {
                static std::vector<BlockCoord> neighborAxes = {
                    BlockCoord(1,0,0),
                    BlockCoord(0,0,1),
                    BlockCoord(1,0,0),
                    BlockCoord(0,0,1),
                };

                
                    uint32_t chestID = 13;

                    float diffX = camera->position.x - placePoint.x;
                    float diffZ = camera->position.z - placePoint.z;

                    int direction = 0;

                    if (std::abs(diffX) > std::abs(diffZ)) {
                        // The player is primarily aligned with the X-axis
                        direction = diffX > 0 ? /*Plus X*/1 : /*Minus X*/3;
                    } else {
                        // The player is primarily aligned with the Z-axis
                        direction = diffZ > 0 ? /*Plus Z*/2 : /*Minus Z*/0;
                    }

                    BlockInfo::setDirectionBits(chestID, direction);



                    voxelWorld.userDataMap.at(chunkToReb).insert_or_assign(placePoint, chestID);

                    auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(chunkToReb);
                    if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                        
                        BlockChunk *chunk = chunkIt->second;


                        while(!voxelWorld.deferredChunkQueue.push(chunk)) {

                        }

                    }
                
            } else
            if(selectedBlockID == 11){ //placing  door

                static std::vector<BlockCoord> neighborAxes = {
                    BlockCoord(1,0,0),
                    BlockCoord(0,0,1),
                    BlockCoord(1,0,0),
                    BlockCoord(0,0,1),
                };

                BlockCoord placeAbove = placePoint + BlockCoord(0,1,0);
                BlockCoord placeBelow = placePoint + BlockCoord(0,-1,0);

                bool condition1 = voxelWorld.blockAt(placeAbove) == 0;
                bool condition2 = voxelWorld.blockAt(placeBelow) != 0;

                if(condition1 && condition2) {
                    uint32_t bottomID = 11;
                    uint32_t topID = 11;

                    topID |= DoorInfo::DOORTOP_BITS;

                    float diffX = camera->position.x - placePoint.x;
                    float diffZ = camera->position.z - placePoint.z;

                    int direction = 0;

                    if (std::abs(diffX) > std::abs(diffZ)) {
                        // The player is primarily aligned with the X-axis
                        direction = diffX > 0 ? /*Plus X*/1 : /*Minus X*/3;
                    } else {
                        // The player is primarily aligned with the Z-axis
                        direction = diffZ > 0 ? /*Plus Z*/2 : /*Minus Z*/0;
                    }

                    BlockInfo::setDirectionBits(bottomID, direction);
                    BlockInfo::setDirectionBits(topID, direction);

                    BlockCoord left;
                    BlockCoord right;

                    if(direction == 0 || direction == 1) {
                        left = placePoint - neighborAxes[direction];
                        right = placePoint + neighborAxes[direction];
                    } else {
                        left = placePoint + neighborAxes[direction];
                        right = placePoint - neighborAxes[direction];
                    }

                    uint32_t blockBitsRight = voxelWorld.blockAt(right);
                    uint32_t blockBitsLeft = voxelWorld.blockAt(left);
                    //std::cout << "Block bits left: " << (blockBitsLeft & BlockInfo::BLOCK_ID_BITS) << "\n";
                    //std::cout << "Block bits right: " << (blockBitsRight & BlockInfo::BLOCK_ID_BITS) << "\n";
                    if((blockBitsRight & BlockInfo::BLOCK_ID_BITS) == 11) {
                        //std::cout << "Door to my right! \n";
                        uint32_t neighdir = BlockInfo::getDirectionBits(blockBitsRight);
                        if(neighdir == direction && DoorInfo::getDoorTopBit(blockBitsRight) == 0) {
                            BlockCoord rightUp = right + BlockCoord(0,1,0);
                            uint32_t neighTopBits = voxelWorld.blockAt(rightUp);

                            DoorInfo::setOppositeDoorBits(topID, 1);
                            DoorInfo::setOppositeDoorBits(bottomID, 1);

                            DoorInfo::setOppositeDoorBits(blockBitsRight, 0);
                            DoorInfo::setOppositeDoorBits(neighTopBits, 0);

                            ChunkCoord chunkToReb2(
                            static_cast<int>(std::floor(static_cast<float>(right.x)/voxelWorld.chunkWidth)),
                            static_cast<int>(std::floor(static_cast<float>(right.z)/voxelWorld.chunkWidth)));

                            voxelWorld.userDataMap.at(chunkToReb2).insert_or_assign(right, blockBitsRight);
                            voxelWorld.userDataMap.at(chunkToReb2).insert_or_assign(rightUp, neighTopBits);
                        }
                    }
                    if((blockBitsLeft & BlockInfo::BLOCK_ID_BITS) == 11) {
                        //std::cout << "Door to my left! \n";
                        uint32_t neighdir = BlockInfo::getDirectionBits(blockBitsLeft);
                        if(neighdir == direction && DoorInfo::getDoorTopBit(blockBitsLeft) == 0) {
                            BlockCoord leftUp = left + BlockCoord(0,1,0);
                            uint32_t neighTopBits = voxelWorld.blockAt(leftUp);

                            DoorInfo::setOppositeDoorBits(topID, 0);
                            DoorInfo::setOppositeDoorBits(bottomID, 0);

                            DoorInfo::setOppositeDoorBits(blockBitsLeft, 1);
                            DoorInfo::setOppositeDoorBits(neighTopBits, 1);

                            ChunkCoord chunkToReb2(
                            static_cast<int>(std::floor(static_cast<float>(left.x)/voxelWorld.chunkWidth)),
                            static_cast<int>(std::floor(static_cast<float>(left.z)/voxelWorld.chunkWidth)));

                            voxelWorld.userDataMap.at(chunkToReb2).insert_or_assign(left, blockBitsLeft);
                            voxelWorld.userDataMap.at(chunkToReb2).insert_or_assign(leftUp, neighTopBits);
                        }
                    }


                    voxelWorld.userDataMap.at(chunkToReb).insert_or_assign(placePoint, bottomID);
                    voxelWorld.userDataMap.at(chunkToReb).insert_or_assign(placeAbove, topID);

                    auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(chunkToReb);
                    if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                        
                        BlockChunk *chunk = chunkIt->second;


                        while(!voxelWorld.deferredChunkQueue.push(chunk)) {

                        }

                    }
                }
            } else 
            if (selectedBlockID == 12){
                voxelWorld.userDataMap.at(chunkToReb).insert_or_assign(placePoint, selectedBlockID);

                auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(chunkToReb);
                if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                    
                    BlockChunk *chunk = chunkIt->second;


                    while(!voxelWorld.lightUpdateQueue.push(chunk)) {

                    }

                }
            }else {

                std::set<BlockChunk *> implicated;
                for(BlockCoord& neigh : BlockInfo::neighbors) {
                    auto segIt = voxelWorld.lightMap.find(rayResult.blockHit + neigh);
                    if(segIt != voxelWorld.lightMap.end()) {
                        
                        for(LightRay& ray : segIt->second.rays) {
                            ChunkCoord chunkOfOrigin(
                                std::floor(static_cast<float>(ray.origin.x)/voxelWorld.chunkWidth),
                                std::floor(static_cast<float>(ray.origin.z)/voxelWorld.chunkWidth)
                            );
                            auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(chunkOfOrigin);
                            if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()){
                                implicated.insert(chunkIt->second);
                            }
                                
                        }
                        
                    }
                }
                for(BlockChunk * pointer : implicated) {
                    while(!voxelWorld.lightUpdateQueue.push(pointer)) {

                    }
                }



                voxelWorld.userDataMap.at(chunkToReb).insert_or_assign(placePoint, selectedBlockID);

                auto chunkIt = voxelWorld.takenCareOfChunkSpots.find(chunkToReb);
                if(chunkIt != voxelWorld.takenCareOfChunkSpots.end()) {
                    
                    BlockChunk *chunk = chunkIt->second;


                    while(!voxelWorld.deferredChunkQueue.push(chunk)) {

                    }

                }
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

    if(key == GLFW_KEY_LEFT_SHIFT) 
    {
        if(action == 1 && grounded) {
            leanSpot = camera->position;
            leaning = action;
        } else {
            leaning = 0;
        }
        
    }
    // if(key == GLFW_KEY_KP_SUBTRACT) {
    //     ambientBrightnessMult = std::max(ambientBrightnessMult - 0.01f, 0.0f);

    // } 
    // if(key == GLFW_KEY_KP_ADD) {
    //     ambientBrightnessMult = std::min(ambientBrightnessMult + 0.01f, 1.0f);

    // }


    // if(key == GLFW_KEY_U && action == 1) {
    //     skyColor.r += 0.1;
    //     std::cout << skyColor.r << "f, " << skyColor.g << "f, " << skyColor.b << "f\n";
    // }
    // if(key == GLFW_KEY_J && action == 1) {
    //     skyColor.r -= 0.1;
    //     std::cout << skyColor.r << "f, " << skyColor.g << "f, " << skyColor.b << "f\n";
    // }

    // if(key == GLFW_KEY_I && action == 1) {
    //     skyColor.g += 0.1;
    //     std::cout << skyColor.r << "f, " << skyColor.g << "f, " << skyColor.b << "f\n";
    // }
    // if(key == GLFW_KEY_K && action == 1) {
    //     skyColor.g -= 0.1;
    //     std::cout << skyColor.r << "f, " << skyColor.g << "f, " << skyColor.b << "f\n";
    // }

    // if(key == GLFW_KEY_O && action == 1) {
    //     skyColor.b += 0.1;
    //     std::cout << skyColor.r << "f, " << skyColor.g << "f, " << skyColor.b << "f\n";
    // }
    // if(key == GLFW_KEY_L && action == 1) {
    //     skyColor.b -= 0.1;
    //     std::cout << skyColor.r << "f, " << skyColor.g << "f, " << skyColor.b << "f\n";
    // }


    if(key == GLFW_KEY_1) {
        noHud = action;
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
        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset){
            Game* instance = static_cast<Game*>(glfwGetWindowUserPointer(window));
            if (instance) {
                //instance->frameBufferSizeCallback(w, width, height);
                instance->selectedBlockID = std::max(std::min(static_cast<int>(instance->selectedBlockID + yoffset), static_cast<int>(BlockInfo::texs.size()-1)), 1);
            }
        });
    } else {
        glfwSetKeyCallback(window, NULL);
        glfwSetCursorPosCallback(window, NULL);
        glfwSetFramebufferSizeCallback(window, NULL);
        glfwSetScrollCallback(window, NULL);
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

        GLuint cam_pos_loc = glGetUniformLocation(wireFrameShader->shaderID, "camPos");
    glUniform3f(cam_pos_loc, camera->position.x, camera->position.y, camera->position.z);

    GLuint viewDistLoc = glGetUniformLocation(wireFrameShader->shaderID, "viewDistance");
    glUniform1f(viewDistLoc, viewDistance);

    RayCastResult result = rayCast(voxelWorld.chunkWidth,
    camera->position,
    camera->direction,
    [this](BlockCoord coord){
            uint32_t block = voxelWorld.blockAt(coord);
            return block != 0 && block != 2;
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

            uniform float time;

            float gaussian(float x, float mean, float stdDev) {
                float a = 1.0 / (stdDev * sqrt(2.0 * 3.14159265));
                float b = exp(-pow(x - mean, 2.0) / (2.0 * pow(stdDev, 2.0)));
                return a * b;
            }

            void main()
            {
                gl_Position = vec4(pos, 0.0, 1.0);

                if(elementid <= -67.0f && elementid >= -75.0f) {
                    float peak = 1.0 + (abs(elementid) - 67)/2.0;
                    float radius = 0.45;
                    gl_Position.y += gaussian(time*3.0, peak, radius)/3.5;
                }

                if(elementid == -997.0) {
                    gl_Position.x -= time/16.0f;
                }
                if(elementid == -998.0) {
                    gl_Position.x += time/16.0f;
                }

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
                if(FragColor.a < 0.1) {
                    discard;
                }
                if(clickedOnElement == elementID) {
                    FragColor = vec4(vec3(1.0, 1.0, 1.0) - FragColor.rgb, 1.0);
                } else if(mousedOverElement == elementID) {
                    FragColor = FragColor + vec4(0.3, 0.3, 0.3, 0.0);
                }
                if(elementID == -99.0f) {
                    FragColor = FragColor - vec4(0.5, 0.5, 0.5, 0.0);
                }
                if(elementID == -97.0f) {
                    discard;
                }
                if(elementID == -98.0f) {
                    FragColor = FragColor - vec4(0.3, 0.3, 0.3, 0.0);
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
            uniform vec3 camPos;
            uniform float ambientBrightMult;
            uniform float viewDistance;
            void main()
            {
                

                float ambBright = ambientBrightMult * ambientBright;

                float distance = pow(distance(position, camPos)/(5), 2)/5.0f;
                gl_Position = mvp * vec4(position , 1.0);

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
            uniform float viewDistance;
            uniform float ambientBrightMult;
            uniform float underWater;
            uniform vec3 camDir;

            uniform float sunset;
            uniform float sunrise;

            float similarity(vec3 dir1, vec3 dir2) {
                return (dot(normalize(dir1), normalize(dir2)) + 1.0) * 0.5;
            }
            void main()
            {
                vec4 texColor = texture(ourTexture, TexCoord);
                FragColor = texColor * vec4(vertexColor, 1.0);

                vec3 west = vec3(0.0,0.0,-1.0);
                vec3 east = vec3(0.0,0.0,1.0);

                vec4 fogColor = vec4(0.7, 0.8, 1.0, 1.0) * vec4(ambientBrightMult, ambientBrightMult, ambientBrightMult, 1.0);

                fogColor = mix(fogColor, vec4(1.0, 0.651, 0.0, 1.0), (similarity(camDir, east) * 0.7) * sunrise);
                fogColor = mix(fogColor, vec4(1.0, 0.651, 0.0, 1.0), (similarity(camDir, west) * 0.7) * sunset); 

                float distance = (distance(pos, camPos)/(viewDistance*5.0f))/5.0f;

                if(underWater == 1.0) {
                    fogColor = vec4(0.0, 0.0, 0.6, 1.0) * vec4(ambientBrightMult, ambientBrightMult, ambientBrightMult, 1.0);
                    distance = distance * 10.0;
                }

                

                if(FragColor.a < 0.4) {
                    discard;
                }

                if(FragColor.a < 1.0) {
                    FragColor.a += distance*2.5f;
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
            uniform vec3 camPos;
            uniform float viewDistance;
            void main()
            {

                float distance = pow(distance(position + translation, camPos)/(5), 2)/5.0f;
                gl_Position = mvp * vec4((position + translation), 1.0);

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
    billBoardShader = std::make_unique<Shader>(
        R"glsl(
            #version 330 core

            layout(location = 0) in vec3 vertexPosition; // Quad vertex positions
            layout(location = 1) in float cornerID;    // Corner ID of quad
            layout(location = 2) in vec3 instancePosition; 
            layout(location = 3) in float blockID; 
            layout(location = 4) in float timeCreated;
            layout(location = 5) in float lifetime;
            layout(location = 6) in vec3 destination;
            layout(location = 7) in float gravity;
            layout(location = 8) in float floorAtDest;

            out vec2 tcoord;
            out float pPassed;

            uniform mat4 v;
            uniform mat4 p;
            uniform mat4 m;
            uniform vec3 camPos;
            uniform float time;

            void main() {

                float timePassed = time - timeCreated;
                

                float percentPassed = min(timePassed/3.0f, 1.0f);

                vec3 realPosition = mix(instancePosition, destination, min(1.0f, percentPassed*5.0f));

                realPosition.y = max( floorAtDest, realPosition.y - timePassed * gravity);

                // Calculate the billboard orientation
                vec3 look = normalize(realPosition - camPos); // Direction from camera to quad
                vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), look)); // Right vector
                vec3 up = cross(look, right); // Up vector

                pPassed = min(timePassed/lifetime, 1.0f);


                // Apply billboard transformation
                vec3 billboardedPosition = realPosition + (vertexPosition.x * right + vertexPosition.y * up);

                float distance = pow(distance(instancePosition, camPos)/(5), 2)/5.0f;

                // Transform position to clip space
                gl_Position = p * v * m * vec4(billboardedPosition, 1.0);



                vec2 baseUV = vec2(mod(blockID, 16.0f)/16.0f, 1.0f - floor(blockID/16.0f));

                // Selecting UV based on cornerID
                if (cornerID == 0.0) {
                    tcoord = baseUV;
                } else if (cornerID == 1.0) {
                    tcoord = vec2(baseUV.x + (1.0f/64.0f), baseUV.y);
                } else if (cornerID == 2.0) {
                    tcoord = vec2(baseUV.x + (1.0f/64.0f), baseUV.y - (1.0f/64.0f));
                } else if (cornerID == 3.0) {
                    tcoord = vec2(baseUV.x, baseUV.y - (1.0f/64.0f));
                }
            }
        )glsl",
        R"glsl(
            #version 330 core
            in vec2 tcoord;
            in float pPassed;
            out vec4 FragColor;
            uniform sampler2D ourTexture;

            void main()
            {
                vec4 texColor = texture(ourTexture, tcoord);
                FragColor = texColor;
                if(texColor.a < 0.1) {
                    discard;
                }
                if(pPassed >= 1.0f) {
                    discard;
                }
            }

        )glsl",
        "billBoardShader"
    );
    blockOverlayShader = std::make_unique<Shader>(
        R"glsl(
            #version 330 core
            layout (location = 0) in vec3 position;
            layout (location = 1) in vec2 uv;

            out vec2 TexCoord;

            uniform mat4 mvp;
            uniform vec3 camPos;

            uniform vec3 blockPosition;
            uniform float breakPhase;

            void main()
            {
                
                float distance = pow(distance(blockPosition, camPos)/(5), 2)/5.0f;

                gl_Position = mvp * vec4((position + blockPosition) , 1.0);


                TexCoord = uv + vec2(breakPhase/16.0f, 0);

            }
        )glsl",
        R"glsl(
            #version 330 core
            in vec2 TexCoord;
            out vec4 FragColor;
            uniform sampler2D ourTexture;
            void main()
            {
                vec4 texColor = texture(ourTexture, TexCoord);
                FragColor = texColor;
                if(FragColor.a == 0.0f) {
                    discard;
                }
            }
        )glsl",
        "blockOverlayShader"
    );
}

std::vector<glm::vec3> Game::randomSpotsAroundCube(const glm::vec3& center, int count, float spread) {
    std::vector<glm::vec3> randomVecs;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-spread, spread);

    for (int i = 0; i < count; ++i) {
        glm::vec3 randomVec;
        randomVec.x = center.x + dis(gen);
        randomVec.y = center.y + dis(gen);
        randomVec.z = center.z + dis(gen);
        randomVecs.push_back(randomVec);
    }

    //std::cout << "returned a vec of " << randomVecs.size() << "size" << "\n";

    return randomVecs;
}

void Game::blockBreakParticles(BlockCoord here, int count) {
    glm::vec3 center(here.x, here.y, here.z);
    std::vector<glm::vec3> spots = randomSpotsAroundCube(center, count, 0.3f);

    int blockID = voxelWorld.blockAt(here);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    float time = static_cast<float>(glfwGetTime());
    for(Particle &particle : particleDisplayData) {

        BlockCoord possiblyThisBlock(
            std::round(particle.destination.x),
            std::round(particle.floorAtDest-0.5f),
            std::round(particle.destination.z)
        );
        if(possiblyThisBlock == here) {

            float timePassed = time - particle.timeCreated;
            float percentPassed = std::min(timePassed/3.0f, 1.0f);
            glm::vec3 realPosition = glm::mix(particle.position, particle.destination, std::min(1.0f, percentPassed*5.0f));

            realPosition.y = std::max(particle.floorAtDest, realPosition.y - timePassed * particle.gravity);
            
            float newFloorAtDest = determineFloorBelowHere(center, here);
            particle.floorAtDest = newFloorAtDest;
            particle.position = realPosition;
            particle.destination = realPosition+glm::vec3(0, 0.5, 0);
            particle.timeCreated = time;
            particle.lifetime = std::max(particle.lifetime - timePassed, 0.1f);
        }
    }


    for(glm::vec3 &spot : spots) {
        glm::vec3 dir = (spot - (center - glm::vec3(0.0, 0.7, 0.0)));
        glm::vec3 dest = spot + dir * 3.0f;

        float floorAtDest = determineFloorBelowHere(dest, here);
        float lifetime = 2.0f + dis(gen);
        float timeCreated = static_cast<float>(glfwGetTime());
        float gravity = 8.0f;

        particleDisplayData.push_back(
            Particle {
            glm::vec3(spot.x, spot.y, spot.z), 
            static_cast<float>(blockID), 
            timeCreated,
            lifetime, 
            glm::vec3(dest.x, dest.y, dest.z),
            gravity, 
            floorAtDest
            }
        );

        // std::cout << spot.x << " " <<  spot.y << " " << spot.z << " "  << static_cast<float>(blockID) << " " << timeCreated << " " << lifetime << " " << dest.x << " " <<  dest.y << " " << 
        //  dest.z << " " << gravity << " " <<  floorAtDest << "\n";
    }
    particlesUploaded = false;
}

void Game::splashyParticles(BlockCoord here, int count) {
    glm::vec3 center(here.x, here.y, here.z);
    std::vector<glm::vec3> spots = randomSpotsAroundCube(center, count, 1.0f);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.1f, 0.1f);
    
    float time = static_cast<float>(glfwGetTime());

    for(glm::vec3 &spot : spots) {
        glm::vec3 dest = spot + glm::vec3(0,20,0);

        float floorAtDest = determineFloorBelowHere(dest, here);
        float lifetime = 0.3f + dis(gen);
        float timeCreated = static_cast<float>(glfwGetTime());
        float gravity = 32.0f;

        particleDisplayData.push_back(
            Particle {
            glm::vec3(spot.x, spot.y-1.0f, spot.z), 
            2.0f, 
            timeCreated,
            lifetime, 
            glm::vec3(dest.x, dest.y, dest.z),
            gravity, 
            floorAtDest
            }
        );

        // std::cout << spot.x << " " <<  spot.y << " " << spot.z << " "  << static_cast<float>(blockID) << " " << timeCreated << " " << lifetime << " " << dest.x << " " <<  dest.y << " " << 
        //  dest.z << " " << gravity << " " <<  floorAtDest << "\n";
    }
    particlesUploaded = false;
}







void Game::runPeriodicTick() {
    static float timer = 0.0f;
    if(timer > 2.0f) {
        cleanUpParticleDisplayData();
        timer = 0.0f;
    } else {
        timer += deltaTime;
    }
}

void Game::cleanUpParticleDisplayData() {

    for (int i = 0; i < particleDisplayData.size(); ) {
        float timeCreated = particleDisplayData[i].timeCreated;
        float lifetime = particleDisplayData[i].lifetime;
        if (glfwGetTime() - timeCreated > lifetime) {
            // Erase this particle
            particleDisplayData.erase(particleDisplayData.begin() + i);
        } else {
            // move on to the next particle
            i++;
        }
    }
    particlesUploaded = false;
}

float Game::determineFloorBelowHere(glm::vec3 here, BlockCoord goingAway) {
    BlockCoord hereb(
        std::round(here.x),
        std::round(here.y),
        std::round(here.z)
    );

    while(voxelWorld.blockAt(hereb) == 0 || hereb == goingAway || hereb.y >= goingAway.y) {
        hereb += BlockCoord(0, -1, 0);
    }

    return static_cast<float>(hereb.y)+0.5f;
}

void Game::drawParticles() {
    glBindVertexArray(VAO2);
    glUseProgram(billBoardShader->shaderID);
    glBindTexture(GL_TEXTURE_2D, worldTexture);

    GLuint camPosLoc = glGetUniformLocation(billBoardShader->shaderID, "camPos");
    glUniform3f(camPosLoc, camera->position.x, camera->position.y, camera->position.z);

    GLuint m_loc = glGetUniformLocation(billBoardShader->shaderID, "m");
    glUniformMatrix4fv(m_loc, 1, GL_FALSE, glm::value_ptr(camera->model));
    GLuint v_loc = glGetUniformLocation(billBoardShader->shaderID, "v");
    glUniformMatrix4fv(v_loc, 1, GL_FALSE, glm::value_ptr(camera->view));
    GLuint p_loc = glGetUniformLocation(billBoardShader->shaderID, "p");
    glUniformMatrix4fv(p_loc, 1, GL_FALSE, glm::value_ptr(camera->projection));

    GLuint time_loc = glGetUniformLocation(billBoardShader->shaderID, "time");
    glUniform1f(time_loc, static_cast<float>(glfwGetTime()));

    static GLuint billposvbo = 0;

    if(particleDisplayData.size() > 0) {
       //std::cout << particleDisplayData.size() << "\n";

        if(!particlesUploaded) {
            glDeleteBuffers(1, &billposvbo);
            glGenBuffers(1, &billposvbo);

            bindBillBoardGeometry(billposvbo, particleDisplayData);
            particlesUploaded = true;
        } else {
            bindBillBoardGeometryNoUpload(billposvbo);
        }
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, particleDisplayData.size());
    }
    glBindVertexArray(0);
}



void Game::bindBillBoardGeometry(GLuint billposvbo, std::vector<Particle> &billinstances) {



    if(billqvbo == 0) {
        glGenBuffers(1, &billqvbo);
        static float quadVertices[] = {
            // Positions    // Corner IDs
            -0.1f, -0.1f, 0.0f, 0.0f,  // Corner 0
            0.1f, -0.1f, 0.0f, 1.0f,  // Corner 1
            0.1f,  0.1f, 0.0f, 2.0f,  // Corner 2
            -0.1f,  0.1f, 0.0f, 3.0f   // Corner 3
        }; 
            // Quad vertices
        glBindBuffer(GL_ARRAY_BUFFER, billqvbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // Vertex position attribute
        GLint posAttrib = glGetAttribLocation(billBoardShader->shaderID, "vertexPosition");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // Corner ID attribute
        GLint cornerAttrib = glGetAttribLocation(billBoardShader->shaderID, "cornerID");
        glEnableVertexAttribArray(cornerAttrib);
        glVertexAttribPointer(cornerAttrib, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, billqvbo);

        // Vertex position attribute
        GLint posAttrib = glGetAttribLocation(billBoardShader->shaderID, "vertexPosition");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // Corner ID attribute
        GLint cornerAttrib = glGetAttribLocation(billBoardShader->shaderID, "cornerID");
        glEnableVertexAttribArray(cornerAttrib);
        glVertexAttribPointer(cornerAttrib, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    }


    // Instance positions
    glBindBuffer(GL_ARRAY_BUFFER, billposvbo);
    glBufferData(GL_ARRAY_BUFFER, billinstances.size() * sizeof(Particle), billinstances.data(), GL_STATIC_DRAW);

    GLint inst_attrib = glGetAttribLocation(billBoardShader->shaderID, "instancePosition");

    glEnableVertexAttribArray(inst_attrib);
    glVertexAttribPointer(inst_attrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glVertexAttribDivisor(inst_attrib, 1); // Instanced attribute


    GLint id_attrib = glGetAttribLocation(billBoardShader->shaderID, "blockID");

    glEnableVertexAttribArray(id_attrib);
    glVertexAttribPointer(id_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribDivisor(id_attrib, 1); // Instanced attribute

    GLint timecreated_attrib = glGetAttribLocation(billBoardShader->shaderID, "timeCreated");

    glEnableVertexAttribArray(timecreated_attrib);
    glVertexAttribPointer(timecreated_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(4*sizeof(float)));
    glVertexAttribDivisor(timecreated_attrib, 1); // Instanced attribute

    GLint lifetime_attrib = glGetAttribLocation(billBoardShader->shaderID, "lifetime");

    glEnableVertexAttribArray(lifetime_attrib);
    glVertexAttribPointer(lifetime_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(5*sizeof(float)));
    glVertexAttribDivisor(lifetime_attrib, 1); // Instanced attribute


    GLint destination_attrib = glGetAttribLocation(billBoardShader->shaderID, "destination");

    glEnableVertexAttribArray(destination_attrib);
    glVertexAttribPointer(destination_attrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6*sizeof(float)));
    glVertexAttribDivisor(destination_attrib, 1); // Instanced attribute

    GLint gravity_attrib = glGetAttribLocation(billBoardShader->shaderID, "gravity");

    glEnableVertexAttribArray(gravity_attrib);
    glVertexAttribPointer(gravity_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9*sizeof(float)));
    glVertexAttribDivisor(gravity_attrib, 1); // Instanced attribute

    GLint flooratdest_attrib = glGetAttribLocation(billBoardShader->shaderID, "floorAtDest");

    glEnableVertexAttribArray(flooratdest_attrib);
    glVertexAttribPointer(flooratdest_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(10*sizeof(float)));
    glVertexAttribDivisor(flooratdest_attrib, 1); // Instanced attribute

}

void Game::bindBillBoardGeometryNoUpload(GLuint billposvbo) {

     if(billqvbo == 0) {
        glGenBuffers(1, &billqvbo);
        static float quadVertices[] = {
            // Positions    // Corner IDs
            -0.1f, -0.1f, 0.0f, 0.0f,  // Corner 0
            0.1f, -0.1f, 0.0f, 1.0f,  // Corner 1
            0.1f,  0.1f, 0.0f, 2.0f,  // Corner 2
            -0.1f,  0.1f, 0.0f, 3.0f   // Corner 3
        }; 
            // Quad vertices
        glBindBuffer(GL_ARRAY_BUFFER, billqvbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // Vertex position attribute
        GLint posAttrib = glGetAttribLocation(billBoardShader->shaderID, "vertexPosition");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // Corner ID attribute
        GLint cornerAttrib = glGetAttribLocation(billBoardShader->shaderID, "cornerID");
        glEnableVertexAttribArray(cornerAttrib);
        glVertexAttribPointer(cornerAttrib, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, billqvbo);

        // Vertex position attribute
        GLint posAttrib = glGetAttribLocation(billBoardShader->shaderID, "vertexPosition");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // Corner ID attribute
        GLint cornerAttrib = glGetAttribLocation(billBoardShader->shaderID, "cornerID");
        glEnableVertexAttribArray(cornerAttrib);
        glVertexAttribPointer(cornerAttrib, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    }


    // Instance positions
    glBindBuffer(GL_ARRAY_BUFFER, billposvbo);

    GLint inst_attrib = glGetAttribLocation(billBoardShader->shaderID, "instancePosition");

    glEnableVertexAttribArray(inst_attrib);
    glVertexAttribPointer(inst_attrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glVertexAttribDivisor(inst_attrib, 1); // Instanced attribute


    GLint id_attrib = glGetAttribLocation(billBoardShader->shaderID, "blockID");

    glEnableVertexAttribArray(id_attrib);
    glVertexAttribPointer(id_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribDivisor(id_attrib, 1); // Instanced attribute

    GLint timecreated_attrib = glGetAttribLocation(billBoardShader->shaderID, "timeCreated");

    glEnableVertexAttribArray(timecreated_attrib);
    glVertexAttribPointer(timecreated_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(4*sizeof(float)));
    glVertexAttribDivisor(timecreated_attrib, 1); // Instanced attribute

    GLint lifetime_attrib = glGetAttribLocation(billBoardShader->shaderID, "lifetime");

    glEnableVertexAttribArray(lifetime_attrib);
    glVertexAttribPointer(lifetime_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(5*sizeof(float)));
    glVertexAttribDivisor(lifetime_attrib, 1); // Instanced attribute


    GLint destination_attrib = glGetAttribLocation(billBoardShader->shaderID, "destination");

    glEnableVertexAttribArray(destination_attrib);
    glVertexAttribPointer(destination_attrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6*sizeof(float)));
    glVertexAttribDivisor(destination_attrib, 1); // Instanced attribute

    GLint gravity_attrib = glGetAttribLocation(billBoardShader->shaderID, "gravity");

    glEnableVertexAttribArray(gravity_attrib);
    glVertexAttribPointer(gravity_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9*sizeof(float)));
    glVertexAttribDivisor(gravity_attrib, 1); // Instanced attribute

    GLint flooratdest_attrib = glGetAttribLocation(billBoardShader->shaderID, "floorAtDest");

    glEnableVertexAttribArray(flooratdest_attrib);
    glVertexAttribPointer(flooratdest_attrib, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(10*sizeof(float)));
    glVertexAttribDivisor(flooratdest_attrib, 1); // Instanced attribute

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

void Game::bindBlockOverlayGeometry() {
    GLenum error;
    if(BOVAO == 0) {
        glGenVertexArrays(1, &BOVAO);
    }
    glBindVertexArray(BOVAO);
    glUseProgram(blockOverlayShader->shaderID);
    TextureFace face(0, 15);
    static std::vector<float> faces2 = {

        -0.55f, -0.5f, 0.5f, face.bl.x, face.bl.y,
        -0.55f, -0.5f, -0.5f, face.br.x, face.br.y,
        -0.55f, 0.5f, -0.5f, face.tr.x, face.tr.y,

        -0.55f, 0.5f, -0.5f, face.tr.x, face.tr.y,
        -0.55f, 0.5f, 0.5f, face.tl.x, face.tl.y,
        -0.55f, -0.5f, 0.5f, face.bl.x, face.bl.y,

                0.55f, -0.5f, -0.5f, face.bl.x, face.bl.y,
                0.55f, -0.5f, 0.5f, face.br.x, face.br.y,
                0.55f, 0.5f, 0.5f,face.tr.x, face.tr.y,

                0.55f, 0.5f, 0.5f, face.tr.x, face.tr.y,
                0.55f, 0.5f, -0.5f, face.tl.x, face.tl.y,
                0.55f, -0.5f, -0.5f,face.bl.x, face.bl.y,

        0.5f, -0.5f, 0.55f, face.bl.x, face.bl.y,
        -0.5f, -0.5f, 0.55f,face.br.x, face.br.y,
        -0.5f, 0.5f, 0.55f,face.tr.x, face.tr.y,

        -0.5f, 0.5f, 0.55f, face.tr.x, face.tr.y,
        0.5f, 0.5f, 0.55f, face.tl.x, face.tl.y,
        0.5f, -0.5f, 0.55f, face.bl.x, face.bl.y,

                -0.5f, -0.5f, -0.55f, face.bl.x, face.bl.y,
                0.5f, -0.5f, -0.55f,face.br.x, face.br.y,
                0.5f, 0.5f, -0.55f,face.tr.x, face.tr.y,

                0.5f, 0.5f, -0.55f, face.tr.x, face.tr.y,
                -0.5f, 0.5f, -0.55f, face.tl.x, face.tl.y,
                -0.5f, -0.5f, -0.55f,face.bl.x, face.bl.y,

        -0.5f, 0.55f, -0.5f, face.bl.x, face.bl.y,
        0.5f, 0.55f, -0.5f,face.br.x, face.br.y,
        0.5f, 0.55f, 0.5f,face.tr.x, face.tr.y,

        0.5f, 0.55f, 0.5f, face.tr.x, face.tr.y,
        -0.5f, 0.55f, 0.5f, face.tl.x, face.tl.y,
        -0.5f, 0.55f, -0.5f,face.bl.x, face.bl.y,

                0.5f, -0.55f, -0.5f, face.bl.x, face.bl.y,
                -0.5f, -0.55f, -0.5f,face.br.x, face.br.y,
                -0.5f, -0.55f, 0.5f,face.tr.x, face.tr.y,

                -0.5f, -0.55f, 0.5f, face.tr.x, face.tr.y,
                0.5f, -0.55f, 0.5f, face.tl.x, face.tl.y,
                0.5f, -0.55f, -0.5f,face.bl.x, face.bl.y
    };

        


    glBindBuffer(GL_ARRAY_BUFFER, bovbo);
    glBufferData(GL_ARRAY_BUFFER, faces2.size() * sizeof(float), faces2.data(), GL_STATIC_DRAW);
error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Bind bo geom err (vbov): " << error << std::endl;
    }
    GLint posAttrib = glGetAttribLocation(blockOverlayShader->shaderID, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Bind bo geom err (vbov): " << error << std::endl;
    }
    GLint uvAttrib = glGetAttribLocation(blockOverlayShader->shaderID, "uv");
    glEnableVertexAttribArray(uvAttrib);
    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "Bind bo geom err (vbov): " << error << std::endl;
    }
    glBindVertexArray(0);
}

void Game::bindBlockOverlayGeometryNoUpload() {
    glBindVertexArray(BOVAO);
}

void Game::drawBlockOverlay() {
    glBindVertexArray(BOVAO);
    if(bovbo == 0) {
        glGenBuffers(1, &bovbo);
        
    }
        
    glUseProgram(blockOverlayShader->shaderID);
    glBindTexture(GL_TEXTURE_2D, worldTexture);

    bindBlockOverlayGeometry();
    bindBlockOverlayGeometryNoUpload();
    
    

    if(blockOverlayShowing) {

    static glm::vec3 lastSpot(0,0,0);
    if(currentSelectCube != lastSpot) {
        uint32_t blockIDHere = voxelWorld.blockAt(BlockCoord(currentSelectCube.x, currentSelectCube.y, currentSelectCube.z)) & BlockInfo::BLOCK_ID_BITS;
        blockBreakingTimer = 0.0f;
        lastSpot = currentSelectCube;
        necessaryBlockBreakingTime = BlockInfo::breakTimes[blockIDHere];
    }

    blockOverlayCoord = currentSelectCube;
    
    GLuint mvp_loc = glGetUniformLocation(blockOverlayShader->shaderID, "mvp");

    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(camera->mvp));

    GLuint cam_pos_loc = glGetUniformLocation(blockOverlayShader->shaderID, "camPos");
    //std::cout << camera->position.x << " " << camera->position.y << " " << camera->position.z << "\n";
    glUniform3f(cam_pos_loc, camera->position.x, camera->position.y, camera->position.z);

    GLuint block_pos_loc = glGetUniformLocation(blockOverlayShader->shaderID, "blockPosition");

    glUniform3f(block_pos_loc, blockOverlayCoord.x, blockOverlayCoord.y, blockOverlayCoord.z);

    GLuint break_phase_loc = glGetUniformLocation(blockOverlayShader->shaderID, "breakPhase");

    static float particleTimer = 0.0f;
    
    glUniform1f(break_phase_loc, 
        static_cast<float>(std::floor((blockBreakingTimer/necessaryBlockBreakingTime)*8.0f))
    );

        if(voxelWorld.blockAt(BlockCoord(
        std::round(blockOverlayCoord.x),
        std::round(blockOverlayCoord.y),
        std::round(blockOverlayCoord.z)
        ))) {
        glDrawArrays(GL_TRIANGLES, 0, 36);
        if(blockBreakingTimer < necessaryBlockBreakingTime) {
            blockBreakingTimer += deltaTime;
            if(particleTimer > 0.5f) {
                blockBreakParticles(BlockCoord(
                    std::round(blockOverlayCoord.x),
                    std::round(blockOverlayCoord.y),
                    std::round(blockOverlayCoord.z)
                ), 4);
                particleTimer = 0.0f;
            } else {
                particleTimer += deltaTime;
            }
        } else {
            castBreakRay();
            if(!displayingSelectCube) {
                blockOverlayShowing = false;
            }
            blockBreakingTimer = 0.0f;
        }
        }
    }
    glBindVertexArray(0);
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
    glm::ivec4 baseColor(0, 45, 100, 140);
    glm::ivec2 coord(0,0);
    int startY = 270, startX = 36, squareSize = 18;
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

    glGenTextures(1, &splashTexture2);
    glBindTexture(GL_TEXTURE_2D, splashTexture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/splash2.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture splashtexture2" << std::endl;
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

        const GLchar* vs_src = R"glsl(

            #version 450 core
            out vec2 v_uv;
            uniform float cpitch;
            void main()
            {
                uint idx = gl_VertexID;
                gl_Position = vec4((idx >> 1), idx & 1, 0.0, 0.5) * 4.0 - 1.0;
                v_uv = vec2(gl_Position.xy + 1.0 + (cpitch / 62));
            }

        )glsl";

        const GLchar* fs_src = R"glsl(

            #version 450 core
            uniform vec4 top_color;
            uniform vec4 bot_color;
            uniform float brightMult;
            uniform float sunrise;
            uniform float sunset;
            uniform vec3 camDir;
            in vec2 v_uv;
            out vec4 frag_color;

            float similarity(vec3 dir1, vec3 dir2) {
                return (dot(normalize(dir1), normalize(dir2)) + 1.0) * 0.5;
            }

            void main()
            {
                vec3 east = vec3(0, 0, 1);
                vec3 west = vec3(0, 0, -1);
                vec4 botColor = mix(bot_color * vec4(brightMult, brightMult, brightMult, 1.0f), bot_color, (similarity(camDir, east)) * sunrise);
                botColor = mix(botColor, bot_color, (similarity(camDir, west)) * sunset);
                frag_color = mix(botColor, top_color * vec4(brightMult, brightMult, brightMult, 1.0f), max(min(pow(v_uv.y-0.4, 1.0), 1.0), 0.0));

            }

        )glsl";

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

    GLuint ssloc = glGetUniformLocation(background_shader, "sunset");
    glUniform1f(ssloc, sunsetFactor);

    GLuint srloc = glGetUniformLocation(background_shader, "sunrise");
    glUniform1f(srloc, sunriseFactor);

    GLuint cam_dir_loc = glGetUniformLocation(background_shader, "camDir");
    glUniform3f(cam_dir_loc, camera->direction.x, camera->direction.y, camera->direction.z);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}
