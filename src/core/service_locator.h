#pragma once

#include <memory>
#include <tuple>

#include "core/camera.h"
#include "core/sparse_set.h"
#include "utils/error.h"

namespace ufps
{

class AwaitableManager;
class MeshManager;
class PhysicsSystem;
class TextureManager;
class ThreadPool;
class RenderEntityManager;
class EntityManager;
class LightManager;
class CameraManager;
class DebugLayer;
class AudioManager;

using CameraHandle = SparseSet<Camera>::handle_type;

using Services = std::tuple<
    std::unique_ptr<AwaitableManager>,
    std::unique_ptr<MeshManager>,
    std::unique_ptr<PhysicsSystem>,
    std::unique_ptr<TextureManager>,
    std::unique_ptr<ThreadPool>,
    std::unique_ptr<RenderEntityManager>,
    std::unique_ptr<EntityManager>,
    std::unique_ptr<LightManager>,
    std::unique_ptr<CameraManager>,
    CameraHandle,
    std::unique_ptr<DebugLayer>,
    std::unique_ptr<AudioManager>>;

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

template <>
inline auto service<CameraHandle>() -> CameraHandle &
{
    expect(!!impl::g_services, "g_services not set");
    return std::get<CameraHandle>(*impl::g_services);
}

template <class... Ts>
auto services() -> std::tuple<Ts &...>
{
    return std::tuple<Ts &...>{service<Ts>()...};
}

}
