#pragma once

#include <algorithm>
#include <cstddef>
#include <inplace_vector>
#include <ranges>
#include <unordered_map>

#include "utils/stack_trace_buffer.h"

namespace ufps
{

namespace impl
{

struct StackTraceHasher
{
    auto operator()(const StackTraceBuffer &stack_trace) const -> std::size_t
    {
        return std::ranges::fold_left(
            stack_trace,
            0zu,
            [](auto seed, auto value)
            { return seed ^= std::hash<void *>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2); });
    }
};

}

using StackTraceCounter = std::unordered_map<StackTraceBuffer, std::size_t, impl::StackTraceHasher>;

}
