#pragma once

#include <string>
#include <string_view>

#include "utils/data_buffer.h"

namespace ufps
{

class ResourceLoader
{
  public:
    virtual ~ResourceLoader() = default;

    virtual auto load_string(std::string_view name) -> std::string = 0;
    virtual auto load_data_buffer(std::string_view name) -> DataBuffer = 0;
};

}
