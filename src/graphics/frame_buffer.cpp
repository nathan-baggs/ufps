#include "graphics/frame_buffer.h"

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "graphics/opengl.h"
#include "graphics/texture.h"
#include "third_party/opengl/glext.h"
#include "utils/error.h"

namespace ufps
{
FrameBuffer::FrameBuffer(
    std::vector<const Texture *> colour_textures,
    const Texture *depth_texture,
    const std::string &name)
    : handle_(0u, [](const auto buffer) { ::glDeleteFramebuffers(1u, &buffer); })
    , colour_textures_{colour_textures}
    , depth_texture_{depth_texture}
    , name_{name}
{
    expect(!colour_textures_.empty(), "must have colour textures");
    expect(colour_textures_.size() < 8u, "hit arbitrary colour texture limit");
    expect(
        std::ranges::all_of(
            colour_textures_,
            [&](const auto *e)
            { return e->width() == colour_textures_[0]->width() && e->height() == colour_textures_[0]->height(); }),
        "all colour textures must have same dimensions");

    ::glCreateFramebuffers(1, &handle_);

    for (const auto &[index, colour_tex] : std::views::enumerate(colour_textures_))
    {
        ::glNamedFramebufferTexture(
            handle_, static_cast<::GLenum>(GL_COLOR_ATTACHMENT0 + index), colour_tex->native_handle(), 0);
    }

    ::glNamedFramebufferTexture(handle_, GL_DEPTH_ATTACHMENT, depth_texture_->native_handle(), 0);

    const auto attachments =
        std::views::iota(0zu, colour_textures_.size()) |
        std::views::transform([](auto e) { return static_cast<::GLenum>(GL_COLOR_ATTACHMENT0 + e); }) |
        std::ranges::to<std::vector>();

    ::glNamedFramebufferDrawBuffers(handle_, static_cast<::GLsizei>(attachments.size()), attachments.data());

    expect(
        ::glCheckNamedFramebufferStatus(handle_, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
        "framebuffer is not complete");

    ::glObjectLabel(GL_FRAMEBUFFER, handle_, name_.length(), name_.data());
}

auto FrameBuffer::native_handle() const -> ::GLuint
{
    return handle_;
}

auto FrameBuffer::bind() const -> void
{
    ::glBindFramebuffer(GL_FRAMEBUFFER, handle_);
}

auto FrameBuffer::unbind() const -> void
{
    ::glBindFramebuffer(GL_FRAMEBUFFER, 0u);
}

auto FrameBuffer::width() const -> std::uint32_t
{
    return colour_textures_.front()->width();
}

auto FrameBuffer::height() const -> std::uint32_t
{
    return colour_textures_.front()->height();
}

auto FrameBuffer::colour_textures() const -> std::span<const Texture *const>
{
    return colour_textures_;
}

auto FrameBuffer::name() const -> std::string_view
{
    return name_;
}

}
