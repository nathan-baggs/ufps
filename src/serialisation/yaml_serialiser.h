#pragma once

#include <concepts>
#include <exception>
#include <expected>
#include <meta>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

#include <yaml-cpp/yaml.h>

#include "maths/bounded_number.h"
#include "maths/matrix4.h"
#include "utils/exception.h"
#include "utils/formatter.h"

namespace ufps::yaml
{

namespace impl
{

template <class T>
concept Bounded = requires {
    typename T::type;
    T::min;
    T::max;
};

template <class T>
concept Class = !std::ranges::range<T> && std::is_class_v<T> && !(requires { typename T::handle_type; }) && !Bounded<T>;

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
concept Sparse = requires { typename T::handle_type; };

template <class T>
concept Enum = std::is_enum_v<T>;

template <Class T>
auto do_serialise(const T &obj) -> std::expected<::YAML::Node, std::string>;
auto do_serialise(const Map auto &obj) -> std::expected<::YAML::Node, std::string>;
template <Class T>
auto do_deserialise(const ::YAML::Node &node) -> std::expected<T, std::string>;

auto do_serialise(const BaseType auto &obj) -> std::expected<::YAML::Node, std::string>
{
    return ::YAML::Node{obj};
}

template <Bounded T>
auto do_serialise(const T &obj) -> std::expected<::YAML::Node, std::string>
{
    return ::YAML::Node{*obj};
}

template <Enum T>
auto do_serialise(const T &obj) -> std::expected<::YAML::Node, std::string>
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

    return std::unexpected(std::format("unknown enum: {}", obj));
}

auto do_serialise(const Array auto &obj) -> std::expected<::YAML::Node, std::string>
{
    auto node = ::YAML::Node{};

    for (const auto &e : obj)
    {
        auto inner_object = do_serialise(e);
        if (!inner_object)
        {
            return std::unexpected(inner_object.error());
        }

        node.push_back(std::move(*inner_object));
    }

    return node;
}

auto do_serialise(const Map auto &obj) -> std::expected<::YAML::Node, std::string>
{
    auto node = ::YAML::Node{};

    for (const auto &[k, v] : obj)
    {
        auto inner_object = do_serialise(v);
        if (!inner_object)
        {
            return std::unexpected(inner_object.error());
        }

        node[k] = std::move(*inner_object);
    }

    return node;
}

auto do_serialise(const Sparse auto &obj) -> std::expected<::YAML::Node, std::string>
{
    auto node = ::YAML::Node{};

    for (const auto &e : obj.data())
    {
        auto inner_object = do_serialise(e);
        if (!inner_object)
        {
            return std::unexpected(inner_object.error());
        }

        node.push_back(std::move(*inner_object));
    }

    return node;
}

inline auto do_serialise(const Matrix4 &obj) -> std::expected<::YAML::Node, std::string>
{
    auto node = ::YAML::Node{};
    std::ranges::for_each(obj.data(), [&node](auto e) { node.push_back(e); });

    return node;
}

template <Class T>
auto do_serialise(const T &obj) -> std::expected<::YAML::Node, std::string>
{
    auto node = ::YAML::Node{};
    auto members = ::YAML::Node{};

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto e : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
    {
        auto inner_object = do_serialise(obj.[:e:]);
        if (!inner_object)
        {
            return std::unexpected(inner_object.error());
        }

        members[std::meta::identifier_of(e)] = std::move(*inner_object);
    }

    node[std::meta::identifier_of(^^T)] = members;

    return node;
}

template <BaseType T>
auto do_deserialise(const ::YAML::Node &node) -> std::expected<T, std::string>
{
    return node.as<T>();
}

template <Bounded T>
auto do_deserialise(const ::YAML::Node &node) -> std::expected<typename T::type, std::string>
{
    return node.as<typename T::type>();
}

template <Enum T>
auto do_deserialise(const ::YAML::Node &node) -> std::expected<T, std::string>
{
    const auto enum_value = node.as<std::string>();

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^T)))
    {
        if (std::meta::identifier_of(e) == enum_value)
        {
            return [:e:];
        }
    }

    return std::unexpected(std::format("unknown enum value {} for {}", enum_value, std::meta::identifier_of(^^T)));
}

template <Array T>
auto do_deserialise(const ::YAML::Node &node) -> std::expected<T, std::string>
{
    auto obj = T{};

    for (const auto &e : node)
    {
        auto inner_element = do_deserialise<std::ranges::range_value_t<T>>(e);
        if (!inner_element)
        {
            return std::unexpected(inner_element.error());
        }

        obj.push_back(std::move(*inner_element));
    }

    return obj;
}

template <Map T>
auto do_deserialise(const ::YAML::Node &node) -> std::expected<T, std::string>
{
    auto obj = T{};

    for (const auto &p : node)
    {
        const auto &key = p.first;
        const auto &value = p.second;

        auto deserialised_key = do_deserialise<typename T::key_type>(key);
        if (!deserialised_key)
        {
            return std::unexpected(deserialised_key.error());
        }

        auto deserialised_value = do_deserialise<typename T::mapped_type>(value);
        if (!deserialised_value)
        {
            return std::unexpected(deserialised_value.error());
        }

        obj[std::move(*deserialised_key)] = std::move(*deserialised_value);
    }

    return obj;
}

template <Sparse T>
auto do_deserialise(const ::YAML::Node &node) -> std::expected<T, std::string>
{
    auto obj = T{};

    for (const auto &e : node)
    {
        auto inner_element = do_deserialise<typename T::value_type>(e);
        if (!inner_element)
        {
            return std::unexpected(inner_element.error());
        }

        obj.emplace(std::move(*inner_element));
    }

    return obj;
}

template <Class T>
auto do_deserialise(const ::YAML::Node &node) -> std::expected<T, std::string>
{
    auto obj = T{};

    const auto inner_node = node[std::meta::identifier_of(^^T)];

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto e : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
    {
        using ElementType = typename[:std::meta::type_of(e):];
        auto inner_element = do_deserialise<ElementType>(inner_node[std::meta::identifier_of(e)]);
        if (!inner_element)
        {
            return std::unexpected(inner_element.error());
        }

        obj.[:e:] = std::move(*inner_element);
    }

    return obj;
}

}

auto serialise(const impl::Class auto &obj) -> std::expected<std::string, std::string>
{
    try
    {
        return impl::do_serialise(obj).and_then(
            [](const auto &e) -> std::expected<std::string, std::string>
            {
                auto strm = std::stringstream{};
                strm << e;
                return strm.str();
            });
    }
    catch (const ::YAML::Exception &e)
    {
        return std::unexpected(std::format("{} [{} {} {}]", e.msg, e.mark.pos, e.mark.line, e.mark.column));
    }
    catch (const std::exception &e)
    {
        return std::unexpected(std::format("{}", e.what()));
    }
    catch (...)
    {
        return std::unexpected<std::string>("unknown exception");
    }
}

template <impl::Class T>
auto deserialise(const std::string &yaml) -> std::expected<T, std::string>
{
    try
    {
        const auto node = ::YAML::Load(yaml);
        return impl::do_deserialise<T>(node);
    }
    catch (const ::YAML::Exception &e)
    {
        return std::unexpected(std::format("{} [{} {} {}]", e.msg, e.mark.pos, e.mark.line, e.mark.column));
    }
    catch (const std::exception &e)
    {
        return std::unexpected(std::format("{}", e.what()));
    }
    catch (...)
    {
        return std::unexpected<std::string>("unknown exception");
    }
}

}
