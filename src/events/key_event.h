#pragma once

#include <string>

#include "key.h"

namespace ufps
{

class KeyEvent
{
  public:
    KeyEvent(Key key, KeyState state);

    auto key() const -> Key;
    auto state() const -> KeyState;

    auto operator==(const KeyEvent &) const -> bool = default;

    auto to_string() const -> std::string;

  private:
    Key key_;
    KeyState state_;
};

}
