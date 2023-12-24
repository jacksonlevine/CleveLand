#ifndef SHADER_H
#define SHADER_H
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

class Shader {
public:
    GLuint shaderID;
    std::string vertexSource;
    std::string fragmentSource;
    Shader(const char *vertex, const char *frag, const char *name);
};
#endif