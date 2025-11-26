#pragma once

#include <variant>

#include "key_event.h"
#include "mouse_button_event.h"
#include "mouse_event.h"
#include "stop_event.h"

namespace ufps
{

using Event = std::variant<StopEvent, KeyEvent, MouseEvent, MouseButtonEvent>;

}
