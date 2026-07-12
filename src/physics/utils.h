#pragma once

#include <meta>
#include <ranges>
#include <string>
#include <unordered_map>

#include "physics/jolt.h"
#include "physics/physics_layers.h"
#include "utils/error.h"
#include "utils/formatter.h"

namespace ufps
{

class SimpleBroadPhaseLayer : public ::JPH::BroadPhaseLayerInterface
{
  public:
    auto GetNumBroadPhaseLayers() const -> ::JPH::uint override
    {
        return std::ranges::size(std::meta::enumerators_of(^^PhysicsLayer));
    }

    auto GetBroadPhaseLayer(::JPH::ObjectLayer layer) const -> ::JPH::BroadPhaseLayer override
    {
        return ::JPH::BroadPhaseLayer{static_cast<::JPH::BroadPhaseLayer::Type>(layer)};
    }

  private:
};

class SimpleObjectVsBroadPhaseLayerFilter : public ::JPH::ObjectVsBroadPhaseLayerFilter
{
  public:
    auto ShouldCollide(::JPH::ObjectLayer layer1, ::JPH::BroadPhaseLayer layer2) const -> bool override
    {
        return PhysicsLayer{layer1} == PhysicsLayer::DYNAMIC ||
               PhysicsLayer{layer2.GetValue()} == PhysicsLayer::DYNAMIC;
    }
};

class SimpleObjectLayerPairFilter : public ::JPH::ObjectLayerPairFilter
{
  public:
    auto ShouldCollide(::JPH::ObjectLayer layer1, ::JPH::ObjectLayer layer2) const -> bool override
    {
        return PhysicsLayer{layer1} == PhysicsLayer::DYNAMIC || PhysicsLayer{layer2} == PhysicsLayer::DYNAMIC;
    }
};

}
