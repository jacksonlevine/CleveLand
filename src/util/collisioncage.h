#ifndef COLLISIONCAGE_H
#define COLLISIONCAGE_H
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>
#include <entt/entt.hpp>
#include "chunkcoord.h"
#include <functional>

class BoundingBox
{
public:
    BoundingBox(const glm::vec3 &m_min_corner, const glm::vec3 &m_max_corner, const glm::vec3 &collision_normal);
    BoundingBox(const glm::vec3 &center, const glm::vec3 &collision_normal);
    void set_center(const glm::vec3 &center);
    void set_center(const glm::vec3 &center, float yextent, float xextent);
    bool intersects(const BoundingBox &other) const;
    double BoundingBox::get_penetration(const BoundingBox &other) const;
    glm::vec3 collision_normal;
    glm::vec3 center;

private:
    glm::vec3 m_min_corner;
    glm::vec3 m_max_corner;
};

enum Side
{
    ROOF = 0,
    FLOOR,

    LEFTTOP,
    LEFTBOTTOM,

    RIGHTTOP,
    RIGHTBOTTOM,

    FRONTTOP,
    FRONTBOTTOM,

    BACKTOP,
    BACKBOTTOM,

    BACKRIGHTTOP,
    BACKRIGHTBOTTOM,

    BACKLEFTTOP,
    BACKLEFTBOTTOM,

    FRONTRIGHTTOP,
    FRONTRIGHTBOTTOM,

    FRONTLEFTTOP,
    FRONTLEFTBOTTOM,

    INBOTTOM,
    INTOP,

    BOTTOMFRONTEDGE,
    BOTTOMLEFTEDGE,
    BOTTOMRIGHTEDGE,
    BOTTOMBACKEDGE

};

#define NUMBER_OF_BOXES 24

class CollisionCage
{
public:
    std::vector<Side> colliding;
    std::vector<Side> solid;
    glm::ivec3 position;
    std::array<BoundingBox, NUMBER_OF_BOXES> boxes;
    std::array<double, NUMBER_OF_BOXES> penetration;
    static const glm::vec3 normals[NUMBER_OF_BOXES];
    static const glm::ivec3 positions[NUMBER_OF_BOXES];
    CollisionCage(std::function<bool(BlockCoord)> solidityPredicate);
    void update_position(glm::vec3 &pos);
    void update_solidity();
    void update_colliding(BoundingBox &user);
    void update_readings(glm::vec3 &pos);

private:
    std::function<bool(BlockCoord)> solidityPredicate;
};

#endif