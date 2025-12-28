#include "graphics/sampler.h"

#include <optional>

#include "graphics/opengl.h"
#include "utils/auto_release.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/formatter.h"

namespace
{

auto to_opengl(ufps::FilterType filter_type) -> ::GLenum
{
    switch (filter_type)
    {
        using enum ufps::FilterType;

        case LINEAR_MIPMAP: return GL_LINEAR_MIPMAP_LINEAR;
        case LINEAR: return GL_LINEAR;
        case NEAREST: return GL_NEAREST;
    }

    throw ufps::Exception("unknown filter_type: {}", filter_type);
}

}

namespace ufps
{

Sampler::Sampler(
    FilterType min_filter,
    FilterType mag_filter,
    const std::string &name,
    std::optional<float> anisotropy_samples)
    : handle_{0u, [](auto sampler) { ::glDeleteSamplers(1, &sampler); }}
    , name_{name}
{
    ::glCreateSamplers(1, &handle_);
    ::glObjectLabel(GL_SAMPLER, handle_, name.length(), name.data());

    ::glSamplerParameteri(handle_, GL_TEXTURE_MIN_FILTER, to_opengl(min_filter));
    ::glSamplerParameteri(handle_, GL_TEXTURE_MAG_FILTER, to_opengl(mag_filter));

    if (anisotropy_samples)
    {
        expect(*anisotropy_samples >= 1.0f, "invalid samples: {}", *anisotropy_samples);
        ::glSamplerParameterf(handle_, GL_TEXTURE_MAX_ANISOTROPY_EXT, *anisotropy_samples);
    }
}

auto Sampler::native_handle() const -> ::GLuint
{
    return handle_;
}

auto Sampler::name() const -> std::string
{
    return name_;
}

}
