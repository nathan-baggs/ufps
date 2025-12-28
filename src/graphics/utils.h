#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "graphics/opengl.h"
#include "graphics/texture_data.h"
#include "utils/data_buffer.h"
#include "utils/log.h"

namespace ufps
{
template <class T>
concept IsBuffer = requires(T t, DataBufferView data, std::size_t offset) {
    { t.write(data, offset) };
};

template <class T, IsBuffer Buffer>
auto resize_gpu_buffer(const std::vector<T> &cpu_buffer, Buffer &gpu_buffer, std::string_view name)
{
    const auto buffer_size_bytes = cpu_buffer.size() * sizeof(T);

    if (gpu_buffer.size() <= buffer_size_bytes)
    {
        auto new_size = gpu_buffer.size() * 2zu;
        while (new_size < buffer_size_bytes)
        {
            new_size *= 2zu;
        }

        ufps::log::info("growing {} buffer {} -> {}", name, gpu_buffer.size(), new_size);

        // opengl barrier incase gpu using previous frame
        ::glFinish();

        gpu_buffer = Buffer{new_size, name};
    }
}

auto load_texture(DataBufferView image_data) -> TextureData;

}
