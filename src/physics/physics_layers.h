#pragma once

namespace ufps
{

enum class BroadPhaseLayer
{
    STATIC,
    DYNAMIC,
};

enum class ObjectLayer
{
    WORLD_COLLIDERS,
    LEVEL_GEOMETRY,
    PLAYER,
    ENEMIES,
};

}
