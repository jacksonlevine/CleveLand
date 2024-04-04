#include "fade.h"

Fader::Fader(float* flo, bool initialValue, float speed) {
    fadeThis = flo;
    value = initialValue;
    this->speed = speed;
}

void Fader::up() {
    value = true;
}


void Fader::down() {
    value = false;
}

void Fader::tick() {
    if(value == true && *fadeThis < 1.0f || value == false && *fadeThis > 0.0f) 
    {
        *fadeThis = std::min(1.0f, std::max(*fadeThis + speed * (value ? 0.1f : -0.1f), 0.0f));
    }
}

