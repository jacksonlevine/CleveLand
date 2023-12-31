#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Game::Game() : lastFrame(0), focused(false), camera(nullptr)
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
    initializeShaders();
    initializeTextures();

    voxelWorld.populateChunksAndNuggos(registry);

    goToMainMenu();
}

void Game::draw() {
    static GLuint VAO = 0;
    if(VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    }


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 1.0, 0.5);




    
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
        stepChunkDraw();
        //std::cout << voxelWorld.nuggosToRebuild.size() << "\n";
        //std::cout << "Camera: \n";
        //std::cout << "  Dir: " << camera->direction.x << " " << camera->direction.y << " " << camera->direction.z << "\n";
        //std::cout << "  Pos: " << camera->position.x << " " << camera->position.y << " " << camera->position.z << "\n";
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


    if(voxelWorld.nuggosToRebuild.size() > 0) {
        if(voxelWorld.meshQueueMutex.try_lock()) {
            Nuggo &nuggo = voxelWorld.nuggoPool[voxelWorld.nuggosToRebuild.back()];
            if(!registry.all_of<MeshComponent>(nuggo.me)) {
                MeshComponent m;
                m.length = nuggo.verts.size();
                bindWorldGeometry(
                    m.vbov,
                    m.vbouv,
                    nuggo.verts.data(),
                    nuggo.uvs.data(),
                    nuggo.verts.size(),
                    nuggo.uvs.size()
                );
                //Transparent stuff
                m.tlength = nuggo.tverts.size();
                bindWorldGeometry(
                    m.tvbov,
                    m.tvbouv,
                    nuggo.tverts.data(),
                    nuggo.tuvs.data(),
                    nuggo.tverts.size(),
                    nuggo.tuvs.size()
                );
                registry.emplace<MeshComponent>(nuggo.me, m);
            } else {
                MeshComponent &m = registry.get<MeshComponent>(nuggo.me);
                glDeleteBuffers(1, &m.vbov);
                glDeleteBuffers(1, &m.vbouv);
                glGenBuffers(1, &m.vbov);
                glGenBuffers(1, &m.vbouv);


                m.length = nuggo.verts.size();
                bindWorldGeometry(
                    m.vbov,
                    m.vbouv,
                    nuggo.verts.data(),
                    nuggo.uvs.data(),
                    nuggo.verts.size(),
                    nuggo.uvs.size()
                );
                //Transparent stuff
                glDeleteBuffers(1, &m.tvbov);
                glDeleteBuffers(1, &m.tvbouv);
                glGenBuffers(1, &m.tvbov);
                glGenBuffers(1, &m.tvbouv);

                m.tlength = nuggo.tverts.size();
                bindWorldGeometry(
                    m.tvbov,
                    m.tvbouv,
                    nuggo.tverts.data(),
                    nuggo.tuvs.data(),
                    nuggo.tverts.size(),
                    nuggo.tuvs.size()
                );
            }
            voxelWorld.nuggosToRebuild.pop_back();
            voxelWorld.meshQueueMutex.unlock();

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
    static std::vector<GUIButton> buttons = {
        GUIButton(0.0f, 0.0f, "Save and exit to main menu", 0.0f, 1.0f, [this](){
            
            inGame = false;
            voxelWorld.runChunkThread = false;
            //voxelWorld.chunkUpdateThread.join();
            voxelWorld.saveWorldToFile(currentSingleplayerWorldPath.c_str());
            camera->setFocused(false);
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
    static std::vector<GUIButton> buttons = {
        GUIButton(0.0f, 0.0f, "Singleplayer", 0.0f, 1.0f, [this](){
            this->goToSingleplayerWorldsMenu();
        }),
        GUIButton(0.0f, -0.1f, "Quit Game", 0.0f, 2.0f, [this](){
            glfwSetWindowShouldClose(this->window, GLFW_TRUE);
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

void Game::goToSingleplayerWorld(const char *worldname) {


    voxelWorld.userDataMap.clear();
    voxelWorld.nuggosToRebuild.clear();
    voxelWorld.takenCareOfChunkSpots.clear();
    voxelWorld.nuggoPool.clear();
    voxelWorld.chunks.clear();


    auto meshesView = registry.view<MeshComponent>();
    for(const entt::entity e : meshesView) {
        MeshComponent &m = registry.get<MeshComponent>(e);
        glDeleteBuffers(1, &m.vbov);
        glDeleteBuffers(1, &m.vbouv);
        glDeleteBuffers(1, &m.tvbov);
        glDeleteBuffers(1, &m.tvbouv);
    }

    registry.clear();

    voxelWorld.populateChunksAndNuggos(registry);



    currentGuiButtons = nullptr;

    currentSingleplayerWorldPath = std::string("saves/") + std::string(worldname) + "/world.save";


    if(voxelWorld.saveExists(currentSingleplayerWorldPath.c_str())) {
        voxelWorld.loadWorldFromFile(currentSingleplayerWorldPath.c_str());
    } else {
        voxelWorld.seed = time(NULL);
        voxelWorld.saveWorldToFile(currentSingleplayerWorldPath.c_str());
    }
    
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

    camera->updatePosition();
    draw();

    glfwPollEvents();
    updateTime();

    if(inGame) {
        static float textureAnimInterval = 0.1f;
        static float textureAnimTimer = 0.0f;
        if(textureAnimTimer > textureAnimInterval) {
            textureAnimTimer = 0.0f;
            stepTextureAnim();
        } else {
            textureAnimTimer += deltaTime;
        }
    }

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
}

void Game::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if(!focused) {
        setFocused(true);
        if(inGame) {
            camera->setFocused(true);
        }
    }
    if(action == GLFW_PRESS) {
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
            camera->setFocused(false);
            displayEscapeMenu();
        }

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
            layout (location = 1) in float brightness;
            layout (location = 2) in vec2 uv;
            out vec3 vertexColor;
            out vec2 TexCoord;
            out vec3 pos;
            uniform mat4 mvp;
            void main()
            {
                gl_Position = mvp * vec4(position, 1.0);
                vertexColor = vec3(brightness/16.0, brightness/16.0, brightness/16.0);
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
                if(FragColor.a < 0.4) {
                    discard;
                }
            }
        )glsl",
        "worldShader"
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
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // Vertex brightness attribute
    GLint brightnessAttrib = glGetAttribLocation(worldShader->shaderID, "brightness");
    glEnableVertexAttribArray(brightnessAttrib);
    glVertexAttribPointer(brightnessAttrib, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));


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
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE,  4 * sizeof(float), 0);

    // Vertex brightness attribute
    GLint brightnessAttrib = glGetAttribLocation(worldShader->shaderID, "brightness");
    glEnableVertexAttribArray(brightnessAttrib);
    glVertexAttribPointer(brightnessAttrib, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));


    glBindBuffer(GL_ARRAY_BUFFER, vbouv);

    GLint uvAttrib = glGetAttribLocation(worldShader->shaderID, "uv");
    glEnableVertexAttribArray(uvAttrib);
    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
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
    glm::ivec4 baseColor(0, 40, 254, 180);
    glm::ivec2 coord(0,0);
    int startY = 270, startX = 18, squareSize = 18;
    for(int y = startY; y < startY + squareSize; ++y) {
        for(int x = startX; x < startX + squareSize; ++x) {
            int i = (y * width + x) * chans;
            float addedNoise = std::max(static_cast<float>(voxelWorld.perlin.noise(
                static_cast<float>(coord.x/4.0f), timer, static_cast<float>(coord.y/4.0f))) * 200, -10.0f);

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
}