#pragma once

#include <bitset>
#include <meta>
#include <utility>

#include "events/key.h"
#include "events/key_event.h"
#include "events/mouse_button_event.h"

namespace ufps
{

namespace impl
{

constexpr auto min_max_val() -> std::pair<std::size_t, std::size_t>
{
    auto min = std::numeric_limits<std::size_t>::max();
    auto max = std::size_t{};

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^Key)))
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

constexpr auto to_index(Key k) -> std::size_t
{
    const auto &[min, _] = min_max_val();
    return std::to_underlying(k) - min;
}

}

class InputMap
{
  public:
    constexpr InputMap()
        : delta_x{}
        , delta_y{}
        , mouse_down{false}
        , map_{}
    {
    }

    constexpr auto set(KeyEvent event) -> void
    {
        const auto index = impl::to_index(event.key());
        map_.set(index, event.state() == KeyState::DOWN);
    }

    constexpr auto is_set(Key key) const -> bool
    {
        const auto index = impl::to_index(key);
        return map_[index];
    }

    template <Key... Keys>
    constexpr auto is_any_set() const -> bool
    {
        return (is_set(Keys) || ...);
    }

    constexpr auto operator[](Key key) const -> bool
    {
        return is_set(key);
    }

    float delta_x;

    float delta_y;

    bool mouse_down;

  private:
    std::bitset<impl::size()> map_;
};

}
