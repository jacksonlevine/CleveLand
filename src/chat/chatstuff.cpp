#include "chatstuff.h"

bool PUSH_TO_TALK_ENABLED = false;
bool PUSHING_TO_TALK = false;

std::unordered_map<std::string, uint32_t> knownVoices;


float volumeByProximity(uint32_t id) {
    static float furthest = 20.0f;

    auto playerIt = std::find_if(PLAYERS.begin(), PLAYERS.end(), [id](OtherPlayer& player){
        return player.id == id;
    });

    if(playerIt != PLAYERS.end() ) {
        float dist = glm::distance(CAMERA_POSITION, glm::vec3(playerIt->x, playerIt->y, playerIt->z));
        return std::max(0.0f, 1.0f - (dist / furthest));
    } else {
        return 1.0f;
    }
}