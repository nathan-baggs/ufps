#pragma once

#include <optional>

#include "graphics/mesh_data.h"
#include "graphics/texture_data.h"

namespace ufps
{

struct ModelData
{
    MeshData mesh_data;
    std::optional<TextureData> albedo;
    std::optional<TextureData> normal;
    std::optional<TextureData> specular;
};

}
