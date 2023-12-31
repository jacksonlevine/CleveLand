#ifndef MESHCOMPONENT_H
#define MESHCOMPONENT_H

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

struct MeshComponent {
public:
    GLuint vbov;
    GLuint vbouv;
    int length;

    GLuint tvbov;
    GLuint tvbouv;
    int tlength;
    MeshComponent();
};

#endif