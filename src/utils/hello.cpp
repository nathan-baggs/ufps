#include "hello.h"

#include <string_view>

namespace ufps
{
auto hello() -> std::string_view
{
    return "hello, world!";
}
}
