#pragma once

#include <bitset>
#include <meta>
#include <utility>

#include "events/key.h"
#include "events/key_event.h"

namespace ufps
{

namespace impl
{

constexpr auto min_max_val() -> std::pair<std::size_t, std::size_t>
{
    auto min = std::numeric_limits<std::size_t>::max();
    auto max = std::size_t{};

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^ufps::Key)))
    {
        const auto val = std::to_underlying([:e:]);

        if (max < val)
        {
            max = val;
        }

        if (min > val)
        {
            min = val;
        }
    }

    return std::make_pair(min, max);
}

constexpr auto size() -> std::size_t
{
    const auto &[min, max] = min_max_val();
    return max - min + 1zu;
}

constexpr auto to_index(ufps::Key k) -> std::size_t
{
    const auto &[min, _] = min_max_val();
    return std::to_underlying(k) - min;
}

}

class KeyMap
{
  public:
    constexpr KeyMap()
        : delta_x{}
        , delta_y{}
        , map_{}
    {
    }

    constexpr auto set(ufps::KeyEvent event) -> void
    {
        const auto index = impl::to_index(event.key());
        map_.set(index, event.state() == KeyState::DOWN);
    }

    constexpr auto is_set(ufps::Key key) const -> bool
    {
        const auto index = impl::to_index(key);
        return map_[index];
    }

    constexpr auto operator[](ufps::Key key) const -> bool
    {
        return is_set(key);
    }

    float delta_x;

    float delta_y;

  private:
    std::bitset<impl::size()> map_;
};

}
