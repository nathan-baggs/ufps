#include "utils/decompress.h"

#include <zstd.h>

#include "utils/data_buffer.h"
#include "utils/error.h"
#include "utils/exception.h"

namespace ufps
{

auto decompress(DataBufferView data) -> DataBuffer
{
    const auto decompressed_buffer_size = ::ZSTD_getFrameContentSize(data.data(), data.size_bytes());
    expect(decompressed_buffer_size != ZSTD_CONTENTSIZE_ERROR, "not compressed by zstd");
    expect(decompressed_buffer_size != ZSTD_CONTENTSIZE_UNKNOWN, "cannot get original size");

    auto decompressed_buffer = DataBuffer(decompressed_buffer_size);

    const auto decompressed_size =
        ::ZSTD_decompress(decompressed_buffer.data(), decompressed_buffer.size(), data.data(), data.size_bytes());

    if (::ZSTD_isError(decompressed_size) == 1)
    {
        throw Exception("failed to compress data: {}", ::ZSTD_getErrorName(decompressed_size));
    }

    return decompressed_buffer;
}

}
