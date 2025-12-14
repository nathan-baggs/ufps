#pragma once

#include "graphics/mesh_view.h"
#include "maths/transform.h"

namespace ufps
{

struct Entity
{
    MeshView mesh_view;
    Transform transform;
};

}
