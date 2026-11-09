#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "graphics/model_data.h"
#include "utils/data_buffer.h"

auto load_model(ufps::DataBufferView model_data) -> std::tuple<std::string, std::vector<ufps::ModelData>>;
