#include "resources/embedded_resource_loader.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

#include "resources/resource_loader.h"
#include "utils/data_buffer.h"
#include "utils/error.h"

namespace
{
constexpr const std::uint8_t sample_vertex_shader[] = {
#embed "../../assets/shaders/simple.vert"
};

constexpr const std::uint8_t sample_fragment_shader[] = {
#embed "../../assets/shaders/simple.frag"
};

constexpr const std::uint8_t gbuffer_vertex_shader[] = {
#embed "../../assets/shaders/gbuffer.vert"
};

constexpr const std::uint8_t gbuffer_fragment_shader[] = {
#embed "../../assets/shaders/gbuffer.frag"
};

constexpr const std::uint8_t light_pass_vertex_shader[] = {
#embed "../../assets/shaders/light_pass.vert"
};

constexpr const std::uint8_t light_pass_fragment_shader[] = {
#embed "../../assets/shaders/light_pass.frag"
};

constexpr const std::uint8_t diamond_floor_albedo[] = {
#embed "../../assets/textures/diamond_floor_albedo.png"
};

constexpr const std::uint8_t diamond_floor_normal[] = {
#embed "../../assets/textures/diamond_floor_normal.png"
};

constexpr const std::uint8_t diamond_floor_specular[] = {
#embed "../../assets/textures/diamond_floor_specular.png"
};

constexpr const std::uint8_t corner01_8_8_x[] = {
#embed "../../secret-assets/models/SM_Corner01_8_8_X.fbx"
};

template <class T>
auto to_container(std::span<const std::uint8_t> data) -> T
{
    static_assert(sizeof(typename T::value_type) == 1);

    const auto *ptr = reinterpret_cast<const T::value_type *>(data.data());
    return T{ptr, ptr + data.size()};
}
}

namespace ufps
{

EmbeddedResourceLoader::EmbeddedResourceLoader()
{
    lookup_ = {
        {"models\\SM_Corner01_8_8_X.fbx", corner01_8_8_x},

        {"shaders\\gbuffer.frag", gbuffer_fragment_shader},
        {"shaders\\gbuffer.vert", gbuffer_vertex_shader},
        {"shaders\\light_pass.frag", light_pass_fragment_shader},
        {"shaders\\light_pass.vert", light_pass_vertex_shader},
        {"shaders\\simple.frag", sample_fragment_shader},
        {"shaders\\simple.vert", sample_vertex_shader},

        {"textures\\diamond_floor_albedo.png", diamond_floor_albedo},
        {"textures\\diamond_floor_normal.png", diamond_floor_normal},
        {"textures\\diamond_floor_specular.png", diamond_floor_specular},
    };
}

auto EmbeddedResourceLoader::load_string(std::string_view name) -> std::string
{
    const auto resource = lookup_.find(name);
    expect(resource != std::ranges::cend(lookup_), "resource {} does not exist", name);

    return to_container<std::string>(resource->second);
}

auto EmbeddedResourceLoader::load_data_buffer(std::string_view name) -> DataBuffer
{
    const auto resource = lookup_.find(name);
    expect(resource != std::ranges::cend(lookup_), "resource {} does not exist", name);

    return to_container<DataBuffer>(resource->second);
}
}
