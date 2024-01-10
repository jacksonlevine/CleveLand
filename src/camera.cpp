#include "camera.h"

Camera3D::Camera3D(Game *gs) : gs(gs), focused(false) {
    yaw = 0.0f;
    pitch = 0.0f;
    fov = 90.0f;

    direction = glm::vec3(0.0f, 0.0f, 1.0f);

    position = glm::vec3(0.0f, 40.0f, 0.0f);

    right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
    up = glm::cross(direction, right);

    model = glm::mat4(1.0f);
    projection = 
        glm::perspective(
            glm::radians(fov),
            static_cast<float>(gs->windowWidth)/gs->windowHeight,
            near,
            far
        );

    view = glm::lookAt(position, position + direction, up);
    mvp = projection * view * model;

    velocity = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Camera3D::mouseCallback(GLFWwindow *window, double xpos, double ypos) {


    static double lastMouseX = 0;
    static double lastMouseY = 0;

    if(firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    double xOffset = (xpos - lastMouseX) * gs->mouseSensitivity;
    double yOffset = (lastMouseY - ypos) * gs->mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if(pitch > 89.0f) {
        pitch = 89.0f;
    }
    if(pitch < -89.0f) {
        pitch = -89.0f;
    }

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction = glm::normalize(direction);

    lastMouseX = xpos;
    lastMouseY = ypos;

    right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
    up = glm::cross(direction, right);
}



void Camera3D::updatePosition() {

    if(forwardPressed) {
        velocity += ((glm::vec3(1.0, 0.0, 1.0) * direction) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }
    if(leftPressed) {
        velocity += ((glm::vec3(1.0, 0.0, 1.0) * right) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }
    if(rightPressed) {
        velocity -= ((glm::vec3(1.0, 0.0, 1.0) * right) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }
    if(backPressed) {
        velocity -= ((glm::vec3(1.0, 0.0, 1.0) * direction) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }

    if(upPressed) {
        velocity += (glm::vec3(0.0, 1.0, 0.0) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }
    if(downPressed) {
        velocity -= (glm::vec3(0.0, 1.0, 0.0) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }








    position += velocity;
    velocity /= 2.0f;
    view = glm::lookAt(position, position + direction, up);
    mvp = projection * view * model;
}

glm::vec3 Camera3D::proposePosition() {
    glm::vec3 proposedPosition;

    if(forwardPressed) {
        velocity += (glm::normalize(glm::vec3(1.0, 0.0, 1.0) * direction)  * speedMulitplier ) * static_cast<float>(gs->deltaTime);
    }
    if(leftPressed) {
        velocity += (glm::normalize(glm::vec3(1.0, 0.0, 1.0) * right) * speedMulitplier ) * static_cast<float>(gs->deltaTime);
    }
    if(rightPressed) {
        velocity -= (glm::normalize(glm::vec3(1.0, 0.0, 1.0) * right) * speedMulitplier  ) * static_cast<float>(gs->deltaTime);
    }
    if(backPressed) {
        velocity -= (glm::normalize(glm::vec3(1.0, 0.0, 1.0) * direction) * speedMulitplier  )* static_cast<float>(gs->deltaTime);
    }

    // if(upPressed) {
    //     velocity += (glm::vec3(0.0, 1.0, 0.0) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    // }
    // if(downPressed) {
    //     velocity -= (glm::vec3(0.0, 1.0, 0.0) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    // }
    
    proposedPosition = position + velocity * (static_cast<float>(gs->averageDeltaTime)* 10.0f);
    velocity /= 1.0f + static_cast<float>(gs->averageDeltaTime) * 10.0f;
    return proposedPosition;
}

void Camera3D::goToPosition(glm::vec3 pos) {
    position = pos;
    view = glm::lookAt(position, position + direction, up);
    mvp = projection * view * model;
}



void Camera3D::frameBufferSizeCallback(GLFWwindow *window, int width, int height) {
    projection = 
        glm::perspective(
            glm::radians(fov),
            static_cast<float>(width)/height,
            near,
            far
        );
    this->updatePosition();
}

void Camera3D::setFocused(bool focused) {
    this->focused = focused;
    if(focused) {
        glfwSetInputMode(gs->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(gs->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Camera3D::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(key == forwardKey) {
        forwardPressed = action;
    }
    if(key == leftKey) {
        leftPressed = action;
    }
    if(key == rightKey) {
        rightPressed = action;
    }
    if(key == backKey) {
        backPressed = action;
    }
    if(key == upKey) {
       upPressed = action;
    }
    if(key == downKey) {
        downPressed = action;
    }
}