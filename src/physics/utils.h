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

    auto GetBroadPhaseLayerName(::JPH::BroadPhaseLayer layer) const -> const char * override
    {
        const auto native_layer = PhysicsLayer{layer.GetValue()};

        static const auto lookup = []
        {
            auto result = std::unordered_map<PhysicsLayer, std::string>{};

            template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^PhysicsLayer)))
            {
                result[[:e:]] = std::string(std::meta::identifier_of(e));
            }

            return result;
        }();

        const auto find = lookup.find(native_layer);
        ensure(find != std::ranges::cend(lookup), "could not find layer: {}", native_layer);

        return find->second.c_str();
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
