#pragma once

#include <vector>

#include "graphics/mesh_view.h"
#include "utils/string_map.h"

namespace ufps
{

struct ModelManifest
{
    MeshView mesh_view;
    std::string albedo_texture;
    std::string normal_texture;
    std::string specular_texture;
};

struct ModelManifestDescription
{
    StringMap<std::vector<ModelManifest>> models;
};

struct TextureManifest
{
    std::uint32_t offset;
    std::uint32_t size;
    bool is_srgb;
};

struct TextureManifestDescription
{
    StringMap<TextureManifest> textures;
};
}
