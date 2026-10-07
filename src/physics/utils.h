#pragma once

#include <meta>
#include <ranges>
#include <string>
#include <unordered_map>

#include "physics/jolt.h"
#include "physics/physics_layers.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/formatter.h"

namespace ufps
{

class SimpleBroadPhaseLayer : public ::JPH::BroadPhaseLayerInterface
{
  public:
    auto GetNumBroadPhaseLayers() const -> ::JPH::uint override
    {
        return std::ranges::size(std::define_static_array(std::meta::enumerators_of(^^BroadPhaseLayer)));
    }

    auto GetBroadPhaseLayer(::JPH::ObjectLayer layer) const -> ::JPH::BroadPhaseLayer override
    {
        const auto object_layer = ObjectLayer{layer};

        switch (object_layer)
        {
            using enum ObjectLayer;

            case WORLD_COLLIDERS: [[fallthrough]];
            case LEVEL_GEOMETRY:
                return ::JPH::BroadPhaseLayer{static_cast<::JPH::BroadPhaseLayer::Type>(BroadPhaseLayer::STATIC)};
            case PLAYER: [[fallthrough]];
            case ENEMIES:
                return ::JPH::BroadPhaseLayer{static_cast<::JPH::BroadPhaseLayer::Type>(BroadPhaseLayer::DYNAMIC)};
        }

        throw Exception("unknown object layer: {}", layer);
    }

  private:
};

class SimpleObjectVsBroadPhaseLayerFilter : public ::JPH::ObjectVsBroadPhaseLayerFilter
{
  public:
    auto ShouldCollide(::JPH::ObjectLayer layer1, ::JPH::BroadPhaseLayer) const -> bool override
    {
        const auto object_layer = ObjectLayer{layer1};

        switch (object_layer)
        {
            using enum ObjectLayer;

            case WORLD_COLLIDERS: return true;
            case LEVEL_GEOMETRY: return false;
            case PLAYER: return true;
            case ENEMIES: return false;
        }

        throw Exception("unknown object layer: {}", layer1);
    }
};

class SimpleObjectLayerPairFilter : public ::JPH::ObjectLayerPairFilter
{
  public:
    auto ShouldCollide(::JPH::ObjectLayer layer1, ::JPH::ObjectLayer layer2) const -> bool override
    {
        const auto object_layer1 = ObjectLayer{layer1};
        const auto object_layer2 = ObjectLayer{layer2};

        if (object_layer1 == ObjectLayer::LEVEL_GEOMETRY || object_layer2 == ObjectLayer::LEVEL_GEOMETRY)
        {
            return false;
        }

        return object_layer1 != object_layer2;
    }
};

class CastRayObjectLayerFilter : public ::JPH::ObjectLayerFilter
{
  public:
    auto ShouldCollide(::JPH::ObjectLayer layer) const -> bool override
    {
        const auto object_layer = ObjectLayer{layer};

        return object_layer == ObjectLayer::LEVEL_GEOMETRY || object_layer == ObjectLayer::ENEMIES;
    }
};

class IgnoreLayerDrawFilter : public ::JPH::BodyDrawFilter
{
  public:
    IgnoreLayerDrawFilter(ObjectLayer object_layer)
        : object_layer_{static_cast<::JPH::ObjectLayer>(object_layer)}
    {
    }

    auto ShouldDraw(const ::JPH::Body &body) const -> bool override
    {
        return body.GetObjectLayer() != object_layer_;
    }

  private:
    ::JPH::ObjectLayer object_layer_;
};

}
