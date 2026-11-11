#pragma once

#include <cstddef>
#include <flat_map>
#include <inplace_vector>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "animation/skeleton.h"
#include "graphics/mesh_data.h"
#include "utils/data_buffer.h"

struct ModelData
{
    ufps::MeshData mesh_data;
    std::optional<ufps::Skeleton> skeleton;
};

using VertexBoneMap = std::unordered_map<std::size_t, std::inplace_vector<ufps::Weight, 4zu>>;

auto load_model(ufps::DataBufferView model_data, std::string_view name)
    -> std::tuple<std::string, std::vector<ModelData>>;
