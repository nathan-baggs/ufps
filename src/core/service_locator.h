#pragma once

#include <memory>
#include <tuple>

#include "utils/error.h"

namespace ufps
{

class AwaitableManager;
class PhysicsSystem;

using Services = std::tuple<std::unique_ptr<AwaitableManager>, std::unique_ptr<PhysicsSystem>>;

namespace impl
{

inline Services *g_services{};

}

inline auto set_service(Services *services)
{
    expect(!impl::g_services, "services already set");
    impl::g_services = services;
}

template <class T>
auto service() -> T &
{
    expect(!!impl::g_services, "g_services not set");
    return *std::get<std::unique_ptr<T>>(*impl::g_services);
}

}
