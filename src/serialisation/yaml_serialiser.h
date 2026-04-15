#pragma once

#include <concepts>
#include <meta>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

#include <yaml-cpp/yaml.h>

#include "maths/vector3.h"
#include "utils/exception.h"

namespace ufps::yaml
{

namespace impl
{

template <class T>
concept Class = std::is_aggregate_v<T> && std::is_class_v<T>;

template <class T>
concept BaseType = std::integral<T> || std::floating_point<T> || std::same_as<T, std::string>;

template <class T>
concept Map = std::ranges::range<T> && requires {
    typename T::key_type;
    typename T::mapped_type;
};

template <class T>
concept Array = std::ranges::range<T> && !Map<T> && !std::same_as<T, std::string>;

template <class T>
concept Enum = std::is_enum_v<T>;

template <class T>
concept IsVector3 = std::same_as<T, Vector3>;

template <Class T>
auto do_serialise(const T &obj) -> ::YAML::Node;
auto do_serialise(const Vector3 &obj) -> ::YAML::Node;
template <Class T>
auto do_deserialise(const ::YAML::Node &node) -> T;

auto do_serialise(const BaseType auto &obj) -> ::YAML::Node
{
    return ::YAML::Node{obj};
}

inline auto do_serialise(const Vector3 &obj) -> ::YAML::Node
{
    auto node = ::YAML::Node{};
    node["x"] = obj.x;
    node["y"] = obj.y;
    node["z"] = obj.z;

    return node;
}

template <Enum T>
auto do_serialise(const T &obj) -> ::YAML::Node
{
    auto node = ::YAML::Node{};

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^T)))
    {
        if (obj == [:e:])
        {
            node = std::meta::identifier_of(e);
            return node;
        }
    }

    node = "<unknown>";
    return node;
}

auto do_serialise(const Array auto &obj) -> ::YAML::Node
{
    auto node = ::YAML::Node{};

    for (const auto &e : obj)
    {
        node.push_back(do_serialise(e));
    }

    return node;
}

auto do_serialise(const Map auto &obj) -> ::YAML::Node
{
    auto node = ::YAML::Node{};

    for (const auto &[k, v] : obj)
    {
        node[k] = do_serialise(v);
    }

    return node;
}

template <Class T>
auto do_serialise(const T &obj) -> ::YAML::Node
{
    auto node = ::YAML::Node{};
    auto members = ::YAML::Node{};

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto e : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
    {
        members[std::meta::identifier_of(e)] = do_serialise(obj.[:e:]);
    }

    node[std::meta::identifier_of(^^T)] = members;

    return node;
}

template <BaseType T>
auto do_deserialise(const ::YAML::Node &node) -> T
{
    return node.as<T>();
}

template <IsVector3>
auto do_deserialise(const ::YAML::Node &node) -> Vector3
{
    return {node["x"].as<float>(), node["y"].as<float>(), node["z"].as<float>()};
}

template <Enum T>
auto do_deserialise(const ::YAML::Node &node) -> T
{
    const auto enum_value = node.as<std::string>();

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^T)))
    {
        if (std::meta::identifier_of(e) == enum_value)
        {
            return [:e:];
        }
    }

    throw Exception("unknown enum value {} for {}", enum_value, std::meta::identifier_of(^^T));
}

template <Array T>
auto do_deserialise(const ::YAML::Node &node) -> T
{
    auto obj = T{};

    for (const auto &e : node)
    {
        obj.push_back(do_deserialise<std::ranges::range_value_t<T>>(e));
    }

    return obj;
}

template <Map T>
auto do_deserialise(const ::YAML::Node &node) -> T
{
    auto obj = T{};

    for (const auto &p : node)
    {
        const auto &key = p.first;
        const auto &value = p.second;
        obj[do_deserialise<typename T::key_type>(key)] = do_deserialise<typename T::mapped_type>(value);
    }

    return obj;
}

template <Class T>
auto do_deserialise(const ::YAML::Node &node) -> T
{
    auto obj = T{};

    const auto inner_node = node[std::meta::identifier_of(^^T)];

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto e : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
    {
        using ElementType = typename[:std::meta::type_of(e):];
        obj.[:e:] = do_deserialise<ElementType>(inner_node[std::meta::identifier_of(e)]);
    }

    return obj;
}

}

auto serialise(const impl::Class auto &obj) -> std::string
{
    auto node = impl::do_serialise(obj);

    auto strm = std::stringstream{};
    strm << node;

    return strm.str();
}

template <impl::Class T>
auto deserialise(const std::string &yaml) -> T
{
    const auto node = ::YAML::Load(yaml);
    return impl::do_deserialise<T>(node);
}

}
