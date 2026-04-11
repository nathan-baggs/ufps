#pragma once

#include <optional>
#include <string>

#include "graphics/mesh_data.h"

namespace ufps
{

struct ModelData
{
    MeshData mesh_data;
    std::optional<std::string> albedo;
    std::optional<std::string> normal;
    std::optional<std::string> specular;
};

}
