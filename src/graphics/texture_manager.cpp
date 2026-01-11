#include "graphics/texture_manager.h"

#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

#include "graphics/buffer.h"
#include "graphics/opengl.h"
#include "graphics/texture.h"
#include "graphics/utils.h"
#include "utils/log.h"

namespace ufps
{

TextureManager::TextureManager()
    : gpu_buffer_{sizeof(::GLuint64), "bindless_textures"}
    , cpu_buffer_{}
    , textures_{}
{
}

auto TextureManager::add(Texture texture) -> std::uint32_t
{
    const auto new_index = textures_.size();

    auto &new_tex = textures_.emplace_back(std::move(texture));
    cpu_buffer_.push_back(new_tex.native_handle());

    resize_gpu_buffer(cpu_buffer_, gpu_buffer_, "bindless_textures");

    gpu_buffer_.write(std::as_bytes(std::span{cpu_buffer_.data(), cpu_buffer_.size()}), 0zu);

    return new_index;
}

auto TextureManager::add(std::vector<Texture> textures) -> std::uint32_t
{
    const auto new_index = textures_.size();

    for (const auto &t : textures)
    {
        log::debug("{}", t.native_handle());
    }

    textures_.append_range(std::views::as_rvalue(textures));
    cpu_buffer_ =
        textures_ | std::views::transform([](auto &e) { return e.native_handle(); }) | std::ranges::to<std::vector>();

    for (const auto &t : cpu_buffer_)
    {
        log::debug("{}", t);
    }

    resize_gpu_buffer(cpu_buffer_, gpu_buffer_, "bindless_textures");

    gpu_buffer_.write(std::as_bytes(std::span{cpu_buffer_.data(), cpu_buffer_.size()}), 0zu);

    return new_index;
}

auto TextureManager::native_handle() const -> ::GLuint
{
    return gpu_buffer_.native_handle();
}
}
