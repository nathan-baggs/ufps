#pragma once

#include <concepts>
#include <random>
#include <ranges>
#include <span>

namespace ufps
{

class Random
{
  public:
    Random(std::uint32_t seed = std::random_device{}())
        : generator_{seed}
    {
    }

    template <std::integral T>
    auto rand_int(T min, T max) -> T
    {
        auto dist = std::uniform_int_distribution<T>{min, max};
        return dist(generator_);
    }

    template <std::floating_point T>
    auto rand_real(T min, T max) -> T
    {
        auto dist = std::uniform_real_distribution<T>{min, max};
        return dist(generator_);
    }

    template <std::ranges::range T>
    const auto &rand_element(T &&elements)
    {
        const auto index = rand_int(0zu, std::ranges::size(elements) - 1zu);
        return *(std::ranges::cbegin(elements) + index);
    }

  private:
    std::mt19937 generator_;
};

namespace random
{

inline auto instance() -> Random &
{
    thread_local Random r{};
    return r;
}

template <std::integral T>
auto rand_int(T min, T max) -> T
{
    return instance().rand_int(min, max);
}

template <std::floating_point T>
auto rand_real(T min, T max) -> T
{
    return instance().rand_real(min, max);
}

template <std::ranges::range T>
const auto &rand_element(T &&elements)
{
    return instance().rand_element(elements);
}

}

}
