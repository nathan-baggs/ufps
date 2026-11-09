#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "graphics/mesh_data.h"
#include "utils/data_buffer.h"

struct ModelData
{
    ufps::MeshData mesh_data;
};

auto load_model(ufps::DataBufferView model_data) -> std::tuple<std::string, std::vector<ModelData>>;
