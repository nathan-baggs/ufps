#pragma once

#include <vector>

#include "graphics/entity.h"
#include "graphics/mesh_manager.h"

namespace ufps
{

struct Scene
{
    std::vector<Entity> entities;
    MeshManager &mesh_manager;
};

}
