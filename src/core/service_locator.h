#pragma once

#include <memory>
#include <tuple>

#include "utils/error.h"

namespace ufps
{

class AwaitableManager;
class MeshManager;
class PhysicsSystem;
class TextureManager;
class ThreadPool;

using Services = std::tuple<
    std::unique_ptr<AwaitableManager>,
    std::unique_ptr<MeshManager>,
    std::unique_ptr<PhysicsSystem>,
    std::unique_ptr<TextureManager>,
    std::unique_ptr<ThreadPool>>;

namespace impl
{

inline Services *g_services{};

}

inline auto set_service(Services *services) //
    pre(impl::g_services == nullptr)        //
    post(impl::g_services != nullptr)
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
