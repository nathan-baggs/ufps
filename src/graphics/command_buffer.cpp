#include "graphics/command_buffer.h"

#include <cstddef>
#include <cstdint>
#include <format>
#include <ranges>
#include <span>
#include <string>

#include "core/scene.h"
#include "graphics/multi_buffer.h"
#include "graphics/opengl.h"
#include "graphics/persistent_buffer.h"
#include "graphics/utils.h"
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
    const auto command = scene.entities |
                         std::views::transform(
                             [](const auto &e)
                             {
                                 const auto cmd = IndirectCommand{
                                     .count = e.mesh_view.index_count,
                                     .instance_count = 1u,
                                     .first = e.mesh_view.index_offset,
                                     .base_vertex = static_cast<std::int32_t>(e.mesh_view.vertex_offset),
                                     .base_instance = 0u,
                                 };
                                 return cmd;
                             }) |
                         std::ranges::to<std::vector>();

    const auto command_view =
        DataBufferView{reinterpret_cast<const std::byte *>(command.data()), command.size() * sizeof(IndirectCommand)};

    resize_gpu_buffer(command, command_buffer_, "command_buffer");

    command_buffer_.write(command_view, 0u);

    return command.size();
}

auto CommandBuffer::build(const Entity &entity) -> std::uint32_t
{
    const auto cmd = IndirectCommand{
        .count = entity.mesh_view.index_count,
        .instance_count = 1u,
        .first = entity.mesh_view.index_offset,
        .base_vertex = 0u,
        .base_instance = 0u,
    };
    const auto command_view = std::as_bytes(std::span{&cmd, 1});

    log::debug("count {} first {}", cmd.count, cmd.first);

    resize_gpu_buffer(std::vector<IndirectCommand>{cmd}, command_buffer_, "command_buffer");

    command_buffer_.write(command_view, 0u);

    return 1u;
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
    return std::format("command buffer {} size", command_buffer_.size());
}

auto CommandBuffer::name() const -> std::string_view
{
    return command_buffer_.name();
}

}
