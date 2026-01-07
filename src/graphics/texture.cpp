#include "graphics/texture.h"

#include <GL/gl.h>
#include <string>

#include "graphics/opengl.h"
#include "graphics/sampler.h"
#include "graphics/texture_data.h"
#include "utils/exception.h"
#include "utils/formatter.h"

namespace
{
auto to_opengl(ufps::TextureFormat format, bool include_size) -> ::GLenum
{
    switch (format)
    {
        using enum ufps::TextureFormat;
        case RED: return include_size ? GL_R8 : GL_RED;
        case RGB: return include_size ? GL_RGB8 : GL_RGB;
        case RGBA: return include_size ? GL_RGBA8 : GL_RGBA;
    }
    throw ufps::Exception("unknown texture format: {}", format);
}

}

namespace ufps
{

Texture::Texture(const TextureData &texture, const std::string &name, const Sampler &sampler)
    : handle_{0z, [](auto texture) { ::glDeleteTextures(1, &texture); }}
    , bindless_handle_{}
    , name_{name}

{
    ::glCreateTextures(GL_TEXTURE_2D, 1, &handle_);
    ::glObjectLabel(GL_TEXTURE, handle_, name.length(), name.data());
    ::glTextureStorage2D(handle_, 1, to_opengl(texture.format, true), texture.width, texture.height);
    ::glTextureSubImage2D(
        handle_,
        0,
        0,
        0,
        texture.width,
        texture.height,
        to_opengl(texture.format, false),
        GL_UNSIGNED_BYTE,
        texture.data.data());

    bindless_handle_ = ::glGetTextureSamplerHandleARB(handle_, sampler.native_handle());
    ::glMakeTextureHandleResidentARB(bindless_handle_);
}

Texture::~Texture()
{
    ::glMakeTextureHandleNonResidentARB(bindless_handle_);
}

auto Texture::native_handle() const -> ::GLuint64
{
    return bindless_handle_;
}

auto Texture::name() const -> std::string
{
    return name_;
};

}
