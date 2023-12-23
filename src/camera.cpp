#include "camera.h"

Camera3D::Camera3D(Game *gs) : gs(gs), focused(false) {
    yaw = 0.0;
    pitch = 0.0;
    fov = 90.0;

    direction = glm::vec3();
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction = glm::normalize(direction);

    position = glm::vec3(0.0, 20.0, 0.0);

    right = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), direction));
    up = glm::cross(direction, right);

    model = glm::mat4(1.0);
    projection = 
        glm::perspective(
            glm::radians(fov),
            static_cast<double>(gs->windowWidth)/gs->windowHeight,
            near,
            far
        );

    view = glm::lookAt(position, position + direction, up);
    mvp = projection * view * model;

    velocity = glm::vec3(0.0, 0.0, 0.0);
}

void Camera3D::mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    static bool firstMouse = true;
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

    if(pitch > 89.0) {
        pitch = 89.0;
    }
    if(pitch < -89.0) {
        pitch = -89.0;
    }

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction = glm::normalize(direction);

    lastMouseX = xpos;
    lastMouseY = ypos;

    right = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), direction));
    up = glm::cross(direction, right);

    view = glm::lookAt(position, position + direction, up);
    mvp = projection * view * model;
}

void Camera3D::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(key == forwardKey) {
        velocity += ((glm::vec3(1.0, 0.0, 1.0) * direction) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }
    if(key == leftKey) {
        velocity += ((glm::vec3(1.0, 0.0, 1.0) * right) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }
    if(key == rightKey) {
        velocity -= ((glm::vec3(1.0, 0.0, 1.0) * right) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }
    if(key == forwardKey) {
        velocity -= ((glm::vec3(1.0, 0.0, 1.0) * direction) * static_cast<float>(gs->deltaTime)) * speedMulitplier;
    }
}

void Camera3D::updatePosition() {
    position += velocity;
    view = glm::lookAt(position, position + direction, up);
    mvp = projection * view * model;
}

void Camera3D::setFocused(bool focused) {
    this->focused = focused;
}

void Camera3D::frameBufferSizeCallback(GLFWwindow *window, int width, int height) {
    projection = 
        glm::perspective(
            glm::radians(fov),
            static_cast<double>(width)/height,
            near,
            far
        );
    mvp = projection * view * model;
}
