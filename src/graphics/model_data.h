#pragma once

#include <optional>

#include "graphics/mesh_data.h"
#include "graphics/texture.h"

namespace ufps
{

struct ModelData
{
    MeshData mesh_data;
    std::optional<Texture> albedo;
    std::optional<Texture> normal;
    std::optional<Texture> specular;
};

}
