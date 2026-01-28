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

constexpr const std::uint8_t metal_plate_02_base_colour[] = {
#embed "../../secret-assets/textures/MetalPlate02_BaseColor.png"
};

constexpr const std::uint8_t corner01_8_8_x[] = {
#embed "../../secret-assets/models/SM_Corner01_8_8_X.fbx"
};

constexpr const std::uint8_t metal_plate_03_base_colour[] = {
#embed "../../secret-assets/textures/MetalPlate03_BaseColor.png"
};

constexpr const std::uint8_t MetalPlate01_BaseColor[] = {
#embed "../../secret-assets/textures/MetalPlate01_BaseColor.png"
};

constexpr const std::uint8_t Details03_BaseColor[] = {
#embed "../../secret-assets/textures/Details03_BaseColor.png"
};

constexpr const std::uint8_t DeatilsBG_BaseColor[] = {
#embed "../../secret-assets/textures/DeatilsBG_BaseColor.png"
};

constexpr const std::uint8_t T_Light_BC[] = {
#embed "../../secret-assets/textures/T_Light_BC.png"
};

constexpr const std::uint8_t Detail02_BaseColor[] = {
#embed "../../secret-assets/textures/Detail02_BaseColor.png"
};

constexpr const std::uint8_t Details01_BaseColor[] = {
#embed "../../secret-assets/textures/Details01_BaseColor.png"
};

constexpr const std::uint8_t Pipes01_BaseColor[] = {
#embed "../../secret-assets/textures/Pipes01_BaseColor.png"
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
        {"textures\\MetalPlate02_BaseColor.png", metal_plate_02_base_colour},
        {"textures\\MetalPlate03_BaseColor.png", metal_plate_03_base_colour},
        {"textures\\MetalPlate01_BaseColor.png", MetalPlate01_BaseColor},
        {"textures\\Details03_BaseColor.png", Details03_BaseColor},
        {"textures\\DeatilsBG_BaseColor.png", DeatilsBG_BaseColor},
        {"textures\\T_Light_BC.png", T_Light_BC},
        {"textures\\Detail02_BaseColor.png", Detail02_BaseColor},
        {"textures\\Details01_BaseColor.png", Details01_BaseColor},
        {"textures\\Pipes01_BaseColor.png", Pipes01_BaseColor},
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
