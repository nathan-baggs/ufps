#include "graphics/texture_manager.h"

#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

#include "graphics/buffer.h"
#include "graphics/opengl.h"
#include "graphics/texture.h"
#include "graphics/utils.h"
#include "utils/error.h"
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
    cpu_buffer_.push_back(new_tex.bindless_handle());

    resize_gpu_buffer(cpu_buffer_, gpu_buffer_);

    gpu_buffer_.write(std::as_bytes(std::span{cpu_buffer_.data(), cpu_buffer_.size()}), 0zu);

    return new_index;
}

auto TextureManager::add(std::vector<Texture> textures) -> std::uint32_t
{
    const auto new_index = textures_.size();

    textures_.append_range(std::views::as_rvalue(textures));
    cpu_buffer_ =
        textures_ | std::views::transform([](auto &e) { return e.bindless_handle(); }) | std::ranges::to<std::vector>();

    resize_gpu_buffer(cpu_buffer_, gpu_buffer_);

    gpu_buffer_.write(std::as_bytes(std::span{cpu_buffer_.data(), cpu_buffer_.size()}), 0zu);

    return new_index;
}

auto TextureManager::native_handle() const -> ::GLuint
{
    return gpu_buffer_.native_handle();
}

auto TextureManager::texture(std::uint32_t index) const -> const Texture *
{
    expect(index <= textures_.size(), "index {} out of range", index);
    return std::addressof(textures_[index]);
}

auto TextureManager::textures(const std::vector<std::uint32_t> &indices) const -> std::vector<const Texture *>
{
    return indices | //
           std::views::transform(
               [this](auto i)
               {
                   expect(i <= textures_.size(), "index {} out of range", i);
                   return std::addressof(textures_[i]);
               }) | //
           std::ranges::to<std::vector>();
}
}
