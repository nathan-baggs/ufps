#pragma once

#include <cstddef>
#include <ranges>
#include <string_view>
#include <tuple>
#include <vector>

#include "graphics/opengl.h"
#include "graphics/texture_data.h"
#include "graphics/vertex_data.h"
#include "resources/resource_loader.h"
#include "utils/data_buffer.h"
#include "utils/log.h"

namespace ufps
{
template <class T>
concept IsBuffer = requires(T t, DataBufferView data, std::size_t offset) {
    { t.write(data, offset) };
    { t.name() } -> std::convertible_to<std::string_view>;
};

template <class T, IsBuffer Buffer>
auto resize_gpu_buffer(const std::vector<T> &cpu_buffer, Buffer &gpu_buffer)
{
    const auto buffer_size_bytes = cpu_buffer.size() * sizeof(T);

    if (gpu_buffer.size() <= buffer_size_bytes)
    {
        auto new_size = gpu_buffer.size() * 2zu;
        while (new_size < buffer_size_bytes)
        {
            new_size *= 2zu;
        }

        ufps::log::info("growing {} buffer {} -> {}", gpu_buffer.name(), gpu_buffer.size(), new_size);

        // opengl barrier incase gpu using previous frame
        ::glFinish();

        gpu_buffer = Buffer{new_size, gpu_buffer.name()};
    }
}

auto load_texture(DataBufferView image_data, bool is_srgb) -> TextureData;

}
