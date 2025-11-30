#include "graphics/buffer.h"

#include <cstddef>
#include <cstdint>

#include "graphics/opengl.h"
#include "utils/auto_release.h"
#include "utils/data_buffer.h"
#include "utils/error.h"

namespace ufps
{

Buffer::Buffer(std::uint32_t size)
    : buffer_{0u, [](auto buffer) { ::glDeleteBuffers(1, &buffer); }}
    , size_{size}
{
    ::glCreateBuffers(1, &buffer_);
    ::glNamedBufferStorage(buffer_, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

auto Buffer::write(DataBufferView data, std::size_t offset) const -> void
{
    expect(size_ >= data.size_bytes() + offset, "buffer too small");
    ::glNamedBufferSubData(buffer_, offset, data.size(), data.data());
}

auto Buffer::native_handle() const -> ::GLuint
{
    return buffer_;
}

}
