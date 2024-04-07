#ifndef MOBTYPE_H
#define MOBTYPE_H

#include <vector>
#include <GLFW/glfw3.h>

enum MobType {
    Player,
    Normal
};

extern GLuint mobVBOs[2];
extern GLuint mobPosVBOs[2];
extern std::vector<std::vector<float>> mobVerts;


#endif