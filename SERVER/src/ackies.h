#ifndef ACKIES_H
#define ACKIES_H

#include <vector>
#include <unordered_map>
#include "uniqueid.h"

struct Ack {
    Goose word;
};


struct AckCentral {
    AckCentral() = default;
    std::unordered_map<Goose, std::vector<int>> library;
    std::unordered_map<Goose, std::vector<char>> archives;
};



#endif