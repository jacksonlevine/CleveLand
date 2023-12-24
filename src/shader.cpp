#include "shader.h"

Shader::Shader(const char *vert, const char *frag, const char *name) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vert, NULL);
    glCompileShader(vertexShader);
    glShaderSource(fragmentShader, 1, &frag, NULL);
    glCompileShader(fragmentShader);
    GLint success;
    char errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << name << " vert shade comp error:" << errorLog << '\n';
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << name << " frag shade comp error:" << errorLog << '\n';
    }
    shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);
    glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << name << " shader program link error:" << errorLog << '\n';
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}