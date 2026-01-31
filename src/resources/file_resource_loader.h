#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "resources/resource_loader.h"
#include "utils/data_buffer.h"

namespace ufps
{

class FileResourceLoader : public ResourceLoader
{
  public:
    FileResourceLoader(const std::vector<std::filesystem::path> &roots);
    ~FileResourceLoader() override = default;

    auto load_string(std::string_view name) -> std::string override;
    auto load_data_buffer(std::string_view name) -> DataBuffer override;

  private:
    std::vector<std::filesystem::path> roots_;
};

}
