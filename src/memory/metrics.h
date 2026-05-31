#pragma once

#include <atomic>
#include <cstddef>

namespace ufps
{

struct Metrics
{
    std::atomic<std::size_t> total_allocation_count;
    std::atomic<std::size_t> live_allocation_count;
    std::atomic<std::size_t> total_allocated_bytes;
    std::atomic<std::size_t> live_allocated_bytes;
    std::atomic<std::size_t> frame_allocated_bytes;
};

struct MetricsSnapshot
{
    std::size_t total_allocation_count;
    std::size_t live_allocation_count;
    std::size_t total_allocated_bytes;
    std::size_t live_allocated_bytes;
    std::size_t frame_allocated_bytes;
};

constinit inline auto g_metrics = Metrics{};

inline auto metrics() -> MetricsSnapshot
{
    return {
        .total_allocation_count = g_metrics.total_allocation_count.load(std::memory_order_relaxed),
        .live_allocation_count = g_metrics.live_allocation_count.load(std::memory_order_relaxed),
        .total_allocated_bytes = g_metrics.total_allocated_bytes.load(std::memory_order_relaxed),
        .live_allocated_bytes = g_metrics.live_allocated_bytes.load(std::memory_order_relaxed),
        .frame_allocated_bytes = g_metrics.frame_allocated_bytes.load(std::memory_order_relaxed),
    };
}

}
