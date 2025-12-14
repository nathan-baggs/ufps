#pragma once

#include <vector>

#include "core/camera.h"
#include "graphics/entity.h"
#include "graphics/material_manager.h"
#include "graphics/mesh_manager.h"

namespace ufps
{

struct Scene
{
    std::vector<Entity> entities;
    MeshManager &mesh_manager;
    MaterialManager &material_manager;
    Camera camera;
};

}
