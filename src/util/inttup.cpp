#include "inttup.h"

IntTup::IntTup()
{
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

void IntTup::set(int nx, int ny, int nz)
{
    this->x = nx;
    this->y = ny;
    this->z = nz;
}

void IntTup::set(int nx, int nz)
{
    this->x = nx;
    this->y = 0;
    this->z = nz;
}

bool IntTup::operator==(const IntTup& other) const {
    return (x == other.x) && (y == other.y) && (z == other.z);
}

bool IntTup::operator!=(const IntTup& other) const {
    return (x != other.x) || (y != other.y) || (z != other.z);
}

IntTup& IntTup::operator+=(const IntTup& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

IntTup& IntTup::operator=(const IntTup& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        z = other.z;
    }
    return *this;
}

IntTup operator+(IntTup first,
                 const IntTup& second)
{
    first.x += second.x;
    first.y += second.y;
    first.z += second.z;

    return first;
}

IntTup::IntTup(int x, int y, int z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

IntTup::IntTup(int x, int z)
{
    this->x = x;
    this->y = 0;
    this->z = z;
}

std::size_t IntTupHash::operator()(const IntTup& tup) const {
    std::size_t hash = 0;
    hash ^= std::hash<int> {}(tup.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<int> {}(tup.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<int> {}(tup.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}