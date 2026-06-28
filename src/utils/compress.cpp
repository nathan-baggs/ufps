#include "utils/compress.h"

#include <zstd.h>

#include "utils/data_buffer.h"
#include "utils/exception.h"

namespace ufps
{

auto compress(DataBufferView data) -> std::expected<DataBuffer, std::string>
{
    const auto compressed_buffer_size = ::ZSTD_compressBound(data.size_bytes());
    auto compressed_buffer = DataBuffer(compressed_buffer_size);

    const auto compressed_size =
        ::ZSTD_compress(compressed_buffer.data(), compressed_buffer.size(), data.data(), data.size_bytes(), 1);

    if (::ZSTD_isError(compressed_size) == 1)
    {
        return std::unexpected(std::format("{}", ::ZSTD_getErrorName(compressed_size)));
    }

    compressed_buffer.resize(compressed_size);

    return compressed_buffer;
}

}
