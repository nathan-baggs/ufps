#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "resources/resource_loader.h"
#include "utils/data_buffer.h"

namespace ufps
{

class FileResourceLoader : public ResourceLoader
{
  public:
    FileResourceLoader(const std::filesystem::path &root);
    ~FileResourceLoader() override = default;

    auto load_string(std::string_view name) -> std::string override;
    auto load_data_buffer(std::string_view name) -> DataBuffer override;

  private:
    std::filesystem::path root_;
};

}
