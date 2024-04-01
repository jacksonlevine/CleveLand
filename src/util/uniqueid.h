#ifndef UNIQUEID_H
#define UNIQUEID_H
#include <mutex>

typedef uint8_t Goose;
class UniqueID {
public:
    UniqueID() = default;
    Goose getID();
private:
    Goose goose;
    std::mutex lock;
};

#endif