#include "concurrency/cond_var.h"

namespace ufps
{
auto CondVar::notify_one() -> void
{
    cv_.notify_one();
}

auto CondVar::notify_all() -> void
{
    cv_.notify_all();
}

}
