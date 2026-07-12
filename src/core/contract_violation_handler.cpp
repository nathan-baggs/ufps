#include <contracts>
#include <stacktrace>

#include "utils/log.h"

auto handle_contract_violation(const std::contracts::contract_violation &cv) -> void;
auto handle_contract_violation(const std::contracts::contract_violation &cv) -> void
{
    ufps::log::error("contract violation {}", cv.comment());
    ufps::log::error("{}", std::stacktrace::current(2));
}
