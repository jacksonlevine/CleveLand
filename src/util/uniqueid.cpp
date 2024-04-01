#include "uniqueid.h"


uint8_t UniqueID::getID() {
    lock.lock();
    float id = goose;
    goose = (goose + 1) % 255;
    lock.unlock();
    return id;
}