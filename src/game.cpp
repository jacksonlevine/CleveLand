#include "game.h"

Game::Game() : lastFrame(0), camera(Camera3D(this)) {
    windowWidth = 1280;
    windowHeight = 720;
    mouseSensitivity = 0.1;
}

void Game::updateTime() {
    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Game::runStep() {
    camera.updatePosition();
    updateTime();
}

void Game::frameBufferSizeCallback(GLFWwindow *window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, windowWidth, windowHeight);
    camera.frameBufferSizeCallback(window, windowWidth, windowHeight);
}

void Game::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {

}

void Game::mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    if(camera.focused) {
        camera.mouseCallback(window, xpos, ypos);
    }
}

void Game::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(camera.focused) {
        camera.keyCallback(window, key, scancode, action, mods);
    }
    if(key == GLFW_KEY_ESCAPE) {
        setFocused(false);
    }
}

void Game::setFocused(bool focused) {
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
    } else {
        glfwSetKeyCallback(window, NULL);
        glfwSetCursorPosCallback(window, NULL);
        glfwSetMouseButtonCallback(window, NULL);
    }
}