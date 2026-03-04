#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "graphics/mesh_view.h"
#include "maths/transform.h"

namespace ufps
{

struct SubMesh
{
    MeshView mesh_view;
    std::uint32_t material_index;
};

struct Entity
{
    std::string name;
    std::vector<SubMesh> sub_meshes;
    Transform transform;
};

}
