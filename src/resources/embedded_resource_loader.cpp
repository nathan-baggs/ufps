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

constexpr const std::uint8_t vertex_data_bin[] = {
#embed "../../build/build_assets/blobs/vertex_data.bin"
};

constexpr const std::uint8_t index_data_bin[] = {
#embed "../../build/build_assets/blobs/index_data.bin"
};

constexpr const std::uint8_t model_manifest_yaml[] = {
#embed "../../build/build_assets/configs/model_manifest.yaml"
};

constexpr const std::uint8_t texture_data_bin[] = {
#embed "../../build/build_assets/blobs/texture_data.bin"
};

constexpr const std::uint8_t texture_manifest_yaml[] = {
#embed "../../build/build_assets/configs/texture_manifest.yaml"
};

constexpr const std::uint8_t average_luminance_comp[] = {
#embed "../../assets/shaders/average_luminance.comp"
};

constexpr const std::uint8_t debug_light_frag[] = {
#embed "../../assets/shaders/debug_light.frag"
};

constexpr const std::uint8_t debug_light_vert[] = {
#embed "../../assets/shaders/debug_light.vert"
};

constexpr const std::uint8_t gbuffer_frag[] = {
#embed "../../assets/shaders/gbuffer.frag"
};

constexpr const std::uint8_t gbuffer_vert[] = {
#embed "../../assets/shaders/gbuffer.vert"
};

constexpr const std::uint8_t light_pass_frag[] = {
#embed "../../assets/shaders/light_pass.frag"
};

constexpr const std::uint8_t light_pass_vert[] = {
#embed "../../assets/shaders/light_pass.vert"
};

constexpr const std::uint8_t line_frag[] = {
#embed "../../assets/shaders/line.frag"
};

constexpr const std::uint8_t line_vert[] = {
#embed "../../assets/shaders/line.vert"
};

constexpr const std::uint8_t luminance_histogram_comp[] = {
#embed "../../assets/shaders/luminance_histogram.comp"
};

constexpr const std::uint8_t simple_frag[] = {
#embed "../../assets/shaders/simple.frag"
};

constexpr const std::uint8_t simple_vert[] = {
#embed "../../assets/shaders/simple.vert"
};

constexpr const std::uint8_t ssao_frag[] = {
#embed "../../assets/shaders/ssao.frag"
};

constexpr const std::uint8_t ssao_blur_frag[] = {
#embed "../../assets/shaders/ssao_blur.frag"
};

constexpr const std::uint8_t ssao_vert[] = {
#embed "../../assets/shaders/ssao.vert"
};

constexpr const std::uint8_t tone_map_frag[] = {
#embed "../../assets/shaders/tone_map.frag"
};

constexpr const std::uint8_t tone_map_vert[] = {
#embed "../../assets/shaders/tone_map.vert"
};

constexpr const std::uint8_t bloom_downsample_frag[] = {
#embed "../../assets/shaders/bloom_downsample.frag"
};

constexpr const std::uint8_t bloom_downsample_vert[] = {
#embed "../../assets/shaders/bloom_downsample.vert"
};

constexpr const std::uint8_t bloom_mix_frag[] = {
#embed "../../assets/shaders/bloom_mix.frag"
};

constexpr const std::uint8_t bloom_mix_vert[] = {
#embed "../../assets/shaders/bloom_mix.vert"
};

constexpr const std::uint8_t bloom_upsample_frag[] = {
#embed "../../assets/shaders/bloom_upsample.frag"
};

constexpr const std::uint8_t bloom_upsample_vert[] = {
#embed "../../assets/shaders/bloom_upsample.vert"
};

constexpr const std::uint8_t chromatic_aberration_frag[] = {
#embed "../../assets/shaders/chromatic_aberration.frag"
};

constexpr const std::uint8_t chromatic_aberration_vert[] = {
#embed "../../assets/shaders/chromatic_aberration.vert"
};

constexpr const std::uint8_t to_the_space_wav[] = {
#embed "../../secret-assets/sounds/ToTheSpace.wav"
};

constexpr const std::uint8_t specter_bullet_wav[] = {
#embed "../../secret-assets/sounds/Specter Bullet.wav"
};

constexpr const std::uint8_t red_planet_wav[] = {
#embed "../../secret-assets/sounds/RedPlanet.wav"
};

constexpr const std::uint8_t ambience_space_station_wav[] = {
#embed "../../secret-assets/sounds/Ambience_Space Station.wav"
};

constexpr const std::uint8_t low_metal_mono_01_wav[] = {
#embed "../../secret-assets/sounds/LowMetal_Mono_01.wav"
};

constexpr const std::uint8_t scene_config[] = {
#embed "../../scene.yaml"
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
        {"blobs\\index_data.bin", std::span{index_data_bin, sizeof(index_data_bin)}},
        {"blobs\\texture_data.bin", std::span{texture_data_bin, sizeof(texture_data_bin)}},
        {"blobs\\vertex_data.bin", std::span{vertex_data_bin, sizeof(vertex_data_bin)}},
        {"configs\\model_manifest.yaml", std::span{model_manifest_yaml, sizeof(model_manifest_yaml)}},
        {"configs\\scene.yaml", std::span{scene_config, sizeof(scene_config)}},
        {"configs\\texture_manifest.yaml", std::span{texture_manifest_yaml, sizeof(texture_manifest_yaml)}},
        {"shaders\\average_luminance.comp", std::span{average_luminance_comp, sizeof(average_luminance_comp)}},
        {"shaders\\debug_light.frag", std::span{debug_light_frag, sizeof(debug_light_frag)}},
        {"shaders\\debug_light.vert", std::span{debug_light_vert, sizeof(debug_light_vert)}},
        {"shaders\\gbuffer.frag", std::span{gbuffer_frag, sizeof(gbuffer_frag)}},
        {"shaders\\gbuffer.vert", std::span{gbuffer_vert, sizeof(gbuffer_vert)}},
        {"shaders\\light_pass.frag", std::span{light_pass_frag, sizeof(light_pass_frag)}},
        {"shaders\\light_pass.vert", std::span{light_pass_vert, sizeof(light_pass_vert)}},
        {"shaders\\line.frag", std::span{line_frag, sizeof(line_frag)}},
        {"shaders\\line.vert", std::span{line_vert, sizeof(line_vert)}},
        {"shaders\\luminance_histogram.comp", std::span{luminance_histogram_comp, sizeof(luminance_histogram_comp)}},
        {"shaders\\simple.frag", std::span{simple_frag, sizeof(simple_frag)}},
        {"shaders\\simple.vert", std::span{simple_vert, sizeof(simple_vert)}},
        {"shaders\\ssao.frag", std::span{ssao_frag, sizeof(ssao_frag)}},
        {"shaders\\ssao.vert", std::span{ssao_vert, sizeof(ssao_vert)}},
        {"shaders\\ssao_blur.frag", std::span{ssao_blur_frag, sizeof(ssao_blur_frag)}},
        {"shaders\\tone_map.frag", std::span{tone_map_frag, sizeof(tone_map_frag)}},
        {"shaders\\tone_map.vert", std::span{tone_map_vert, sizeof(tone_map_vert)}},
        {"shaders\\bloom_downsample.frag", std::span{bloom_downsample_frag, sizeof(bloom_downsample_frag)}},
        {"shaders\\bloom_downsample.vert", std::span{bloom_downsample_vert, sizeof(bloom_downsample_vert)}},
        {"shaders\\bloom_mix.frag", std::span{bloom_mix_frag, sizeof(bloom_mix_frag)}},
        {"shaders\\bloom_mix.vert", std::span{bloom_mix_vert, sizeof(bloom_mix_vert)}},
        {"shaders\\bloom_upsample.frag", std::span{bloom_upsample_frag, sizeof(bloom_upsample_frag)}},
        {"shaders\\bloom_upsample.vert", std::span{bloom_upsample_vert, sizeof(bloom_upsample_vert)}},
        {"shaders\\chromatic_aberration.frag", std::span{chromatic_aberration_frag, sizeof(chromatic_aberration_frag)}},
        {"shaders\\chromatic_aberration.vert", std::span{chromatic_aberration_vert, sizeof(chromatic_aberration_vert)}},
        {"sounds\\Ambience_Space Station.wav", std::span{ambience_space_station_wav, sizeof(ambience_space_station_wav)}},
        {"sounds\\LowMetal_Mono_01.wav", std::span{low_metal_mono_01_wav, sizeof(low_metal_mono_01_wav)}},
        {"sounds\\RedPlanet.wav", std::span{red_planet_wav, sizeof(red_planet_wav)}},
        {"sounds\\Specter Bullet.wav", std::span{specter_bullet_wav, sizeof(specter_bullet_wav)}},
        {"sounds\\ToTheSpace.wav", std::span{to_the_space_wav, sizeof(to_the_space_wav)}},
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

auto EmbeddedResourceLoader::resources(std::string_view type) -> std::vector<std::string>
{
    auto prefix = std::string{type};
    prefix += '\\';

    auto result = std::vector<std::string>{};
    for (const auto &entry : lookup_)
    {
        if (entry.first.starts_with(prefix))
        {
            result.push_back(entry.first);
        }
    }

    return result;
}
}
