#pragma once

#include <concepts>
#include <meta>
#include <sstream>
#include <string>
#include <type_traits>

#include <yaml-cpp/yaml.h>

namespace ufps::yaml
{

namespace impl
{

template <class T>
auto serialise(T &obj) -> ::YAML::Node
{
    auto node = ::YAML::Node{};

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto e : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
    {
        node[std::meta::identifier_of(e)] = obj.[:e:];
    }

    return node;
}

template <class T>
struct SerialiseRange;

template <class T>
struct SerialiseRange<std::vector<T>>
{
    auto operator()(::YAML::Node &node, const std::vector<T> &vec) -> void
    {
        for (const auto &v : vec)
        {
            if constexpr (std::integral<T> || std::floating_point<T> || std::same_as<T, std::string>)
            {
                node.push_back(v);
            }
            else
            {
                node.push_back(serialise(v));
            }
        }
    }
};

template <class K, class V>
struct SerialiseRange<std::unordered_map<K, V>>
{
    auto operator()(::YAML::Node &node, const std::unordered_map<K, V> &map) -> void
    {
        for (const auto &[k, v] : map)
        {
            node[k] = v;
        }
    }
};

template <>
struct SerialiseRange<std::string>
{
    auto operator()(::YAML::Node &node, const std::string &str) -> void
    {
        node = str;
    }
};

}

template <class T>
auto serialise(T &&obj) -> std::string
{
    auto node = ::YAML::Node{};
    auto members = ::YAML::Node{};

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto e : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
    {
        if constexpr (std::ranges::range<typename[:std::meta::type_of(e):]>)
        {
            auto vec_node = ::YAML::Node{};
            impl::SerialiseRange<typename[:std::meta::type_of(e):]>{}(vec_node, obj.[:e:]);

            members[std::meta::identifier_of(e)] = std::move(vec_node);
        }
        else
        {
            members[std::meta::identifier_of(e)] = obj.[:e:];
        }
    }

    node[std::meta::identifier_of(^^T)] = members;

    auto strm = std::stringstream{};
    strm << node;

    return strm.str();
}

}
