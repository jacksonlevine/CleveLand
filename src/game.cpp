#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Game::Game() : lastFrame(0), focused(false), camera(nullptr)
{
    camera = new Camera3D(this);
    windowWidth = 1280;
    windowHeight = 720;
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
}

void Game::draw() {
    static GLuint VAO = 0;
    if(VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glUseProgram(menuShader->shaderID);
    glBindTexture(GL_TEXTURE_2D, menuTexture);

    static std::vector<GUIButton> buttons = {
        GUIButton(0.0f, 0.0f, "Test Label", 0.0f, 1.0f),
        GUIButton(0.0f, -0.1f, "Other", 0.0f, 2.0f),
        GUIButton(0.0f, -0.2f, "This thing", 0.0f, 3.0f),
    };

    mousedOverElement = 0.0f;

    for(GUIButton& button : buttons) {
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
    

    glfwSwapBuffers(window);
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
}

void Game::frameBufferSizeCallback(GLFWwindow *window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, windowWidth, windowHeight);
    camera->frameBufferSizeCallback(window, windowWidth, windowHeight);
}

void Game::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if(!focused) {
        setFocused(true);
    }
    if(action == GLFW_PRESS) {
        clickedOnElement = mousedOverElement;
    } else {
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
        setFocused(false);
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
        glfwSetMouseButtonCallback(window, NULL);
        glfwSetFramebufferSizeCallback(window, NULL);
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
                if(mousedOverElement == elementID) {
                    FragColor = FragColor + vec4(0.3, 0.3, 0.3, 0.0);
                }
                if(clickedOnElement == elementID) {
                    FragColor = vec4(vec3(1.0, 1.0, 1.0) - FragColor.rgb, 1.0);
                }
            }
        )glsl",
        "menuShader"
    );
    worldShader = std::make_unique<Shader>(
        R"glsl(
            #version 450 core
            layout (location = 0) in vec3 position;
            layout (location = 1) in vec2 uv;
            out vec3 vertexColor;
            out vec2 TexCoord;
            out vec3 pos;
            uniform mat4 mvp;
            void main()
            {
                gl_Position = mvp * vec4(position, 1.0);
                vertexColor = vec3(1.0, 1.0, 1.0);
                TexCoord = uv;
                pos = position;
            }
        )glsl",
        R"glsl(
            #version 450 core
            in vec3 vertexColor;
            in vec2 TexCoord;
            in vec3 pos;
            out vec4 FragColor;
            uniform sampler2D ourTexture;
            uniform vec3 camPos;
            void main()
            {
                vec4 texColor = texture(ourTexture, TexCoord);
                FragColor = texColor;
                if(FragColor.a < 1.0) {
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
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

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
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbouv);

    GLint uvAttrib = glGetAttribLocation(worldShader->shaderID, "uv");
    glEnableVertexAttribArray(uvAttrib);
    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
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

    glGenTextures(1, &worldTexture);
    glBindTexture(GL_TEXTURE_2D, worldTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("assets/world.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture worldtexture" << std::endl;
    }
    stbi_image_free(data);
}