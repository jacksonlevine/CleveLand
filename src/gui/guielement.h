#ifndef GUIELEMENT_H
#define GUIELEMENT_H
#include "glyphface.h"
#include <vector>
#include "../util/textureface.h"

std::vector<float> createButton(float xOffset, float yOffset, const char *label, float manualWidth);

#endif