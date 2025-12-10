#include "graphics/command_buffer.h"

#include <cstddef>
#include <cstdint>
#include <format>
#include <ranges>
#include <string>

#include "graphics/multi_buffer.h"
#include "graphics/opengl.h"
#include "graphics/persistent_buffer.h"
#include "graphics/scene.h"
#include "utils/log.h"

namespace
{
struct IndirectCommand
{
    std::uint32_t count;
    std::uint32_t instance_count;
    std::uint32_t first;
    std::int32_t base_vertex;
    std::uint32_t base_instance;
};

}

namespace ufps
{

CommandBuffer::CommandBuffer()
    : command_buffer_{1u, "command_buffer"}
{
}

auto CommandBuffer::build(const Scene &scene) -> std::uint32_t
{
    auto base = 0;
    const auto command = scene.entities |
                         std::views::transform(
                             [&base](const auto &e)
                             {
                                 const auto cmd = IndirectCommand{
                                     .count = e.mesh_view.index_count,
                                     .instance_count = 1u,
                                     .first = e.mesh_view.index_offset,
                                     .base_vertex = base,
                                     .base_instance = 0u,
                                 };
                                 base += e.mesh_view.vertex_offset;
                                 return cmd;
                             }) |
                         std::ranges::to<std::vector>();

    const auto command_view =
        DataBufferView{reinterpret_cast<const std::byte *>(command.data()), command.size() * sizeof(IndirectCommand)};

    if (command_view.size_bytes() > command_buffer_.original_size())
    {
        auto new_size = command_buffer_.original_size() * 2zu;
        while (new_size < command_view.size_bytes())
        {
            new_size *= 2zu;
        }

        log::info("growing command buffer {} -> {}", command_buffer_.original_size(), new_size);

        // opengl barrier incase gpu using previous frame
        ::glFinish();

        command_buffer_ = MultiBuffer<PersistentBuffer>{new_size, "mesh_data"};
    }

    command_buffer_.write(command_view, 0u);

    return command.size();
}

auto CommandBuffer::native_handle() const -> ::GLuint
{
    return command_buffer_.buffer().native_handle();
}

auto CommandBuffer::advance() -> void
{
    command_buffer_.advance();
}

auto CommandBuffer::offset_bytes() const -> std::size_t
{
    return command_buffer_.frame_offset_bytes();
}

auto CommandBuffer::to_string() const -> std::string
{
    return std::format("command buffer {} size", command_buffer_.original_size());
}

}
