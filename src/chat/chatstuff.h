#ifndef CHATSTUFF_H
#define CHATSTUFF_H

#include <unordered_map>
#include <string>
#include "../network.h"
#include "../util/cameraposition.h"

extern bool PUSH_TO_TALK_ENABLED;
extern bool PUSHING_TO_TALK;

extern std::unordered_map<std::string, uint32_t> knownVoices;

float volumeByProximity(uint32_t id);

#endif