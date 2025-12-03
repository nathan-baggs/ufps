#include "graphics/persistent_buffer.h"

#include <cstddef>
#include <cstring>
#include <string_view>

#include "graphics/opengl.h"
#include "utils/auto_release.h"
#include "utils/data_buffer.h"
#include "utils/error.h"

namespace ufps
{

PersistentBuffer::PersistentBuffer(std::size_t size, std::string_view name)
    : buffer_{0u, [](auto buffer) { ::glUnmapNamedBuffer(buffer); ::glDeleteBuffers(1, &buffer); }}
    , size_{size}
{
    const auto flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    ::glCreateBuffers(1, &buffer_);
    ::glNamedBufferStorage(buffer_, size, nullptr, GL_DYNAMIC_STORAGE_BIT | flags);
    ::glObjectLabel(GL_BUFFER, buffer_, name.length(), name.data());

    map_ = ::glMapNamedBufferRange(buffer_, 0, size, flags);
}

auto PersistentBuffer::write(DataBufferView data, std::size_t offset) const -> void
{
    expect(size_ >= data.size_bytes() + offset, "buffer too small");
    std::memcpy(reinterpret_cast<std::byte *>(map_) + offset, data.data(), data.size_bytes());
}

auto PersistentBuffer::native_handle() const -> ::GLuint
{
    return buffer_;
}

}
