#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/sub_mesh.h"
#include "graphics/mesh_view.h"
#include "maths/aabb.h"
#include "maths/transform.h"

namespace ufps
{

struct Entity
{
    std::string name;
    std::vector<SubMesh> sub_meshes;
    Transform transform;
};

}
