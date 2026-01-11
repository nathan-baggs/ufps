#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "resources/resource_loader.h"
#include "utils/data_buffer.h"
#include "utils/string_map.h"

namespace ufps
{

class EmbeddedResourceLoader : public ResourceLoader
{
  public:
    EmbeddedResourceLoader();
    ~EmbeddedResourceLoader() override = default;
    auto load_string(std::string_view name) -> std::string override;
    auto load_data_buffer(std::string_view name) -> DataBuffer override;

  private:
    StringMap<std::span<const std::uint8_t>> lookup_;
};

}
