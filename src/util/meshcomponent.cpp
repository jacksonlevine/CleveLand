#include "meshcomponent.h"
MeshComponent::MeshComponent() : length(0) {
    glGenBuffers(1, &this->vbov);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after glGenBuffers vbov: " << error << std::endl;
    }

    glGenBuffers(1, &this->vbouv);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after glGenBuffers vbouv: " << error << std::endl;
    }


        glGenBuffers(1, &this->tvbov);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after glGenBuffers tvbov: " << error << std::endl;
    }

    glGenBuffers(1, &this->tvbouv);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after glGenBuffers tvbouv: " << error << std::endl;
    }
}