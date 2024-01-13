#include "random.h"

//float 0.0 - 1.0
float rando() {
    return static_cast<float>(rand()) / RAND_MAX;
}

//int 0 - 128
int randsmall() {
    return (rand() >> 8);
}
