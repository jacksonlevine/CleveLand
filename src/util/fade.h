#ifndef FADE_H
#define FADE_H

#include <algorithm>

class Fader {
public:
    float* fadeThis;
    bool value;
    void up();
    void down();
    float speed;
    void tick();
    Fader(float* flo, bool initialValue, float speed);
};



#endif