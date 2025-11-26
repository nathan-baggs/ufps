#include "events/key_event.h"

#include <format>

#include "key.h"
#include "utils/formatter.h"

namespace ufps
{

KeyEvent::KeyEvent(Key key, KeyState state)
    : key_(key)
    , state_(state)
{
}

auto KeyEvent::key() const -> Key
{
    return key_;
}

auto KeyEvent::state() const -> KeyState
{
    return state_;
}

auto KeyEvent::to_string() const -> std::string
{
    return std::format("KeyEvent {} {}", key_, state_);
}

}
