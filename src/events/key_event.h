#pragma once

#include <format>
#include <string>

#include "key.h"
#include "utils/formatter.h"

namespace ufps
{

class KeyEvent
{
  public:
    constexpr KeyEvent(Key key, KeyState state);

    constexpr auto key() const -> Key;
    constexpr auto state() const -> KeyState;

    constexpr auto operator==(const KeyEvent &) const -> bool = default;

    constexpr auto to_string() const -> std::string;

  private:
    Key key_;
    KeyState state_;
};

constexpr KeyEvent::KeyEvent(Key key, KeyState state)
    : key_(key)
    , state_(state)
{
}

constexpr auto KeyEvent::key() const -> Key
{
    return key_;
}

constexpr auto KeyEvent::state() const -> KeyState
{
    return state_;
}

constexpr auto KeyEvent::to_string() const -> std::string
{
    return std::format("KeyEvent {} {}", key_, state_);
}
}
