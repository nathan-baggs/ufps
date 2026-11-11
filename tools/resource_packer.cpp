#include <filesystem>
#include <fstream>
#include <ranges>
#include <span>
#include <unordered_map>
#include <unordered_set>

#include <yaml-cpp/yaml.h>

#include "animation/skeleton.h"
#include "core/manifest_descriptions.h"
#include "graphics/utils.h"
#include "resources/file_resource_loader.h"
#include "serialisation/yaml_serialiser.h"
#include "utils/compress.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

#include "assimp_loader.h"

using namespace std::literals;

namespace
{

struct Textures
{
    std::string albedo_texture;
    std::string normal_texture;
    std::string specular_texture;
    std::string ao_texture;
    std::string glossiness_texture;
    std::string emissive_texture;
};

struct ModelTextures
{
    std::unordered_map<std::string, std::vector<Textures>> models;
};

template <class T>
auto write_blob(const T &data, const std::filesystem::path &output_blobs_dir, std::string_view file_name) -> void
{
    const auto compressed_data =
        ufps::compress(std::as_bytes(std::span(std::ranges::data(data), std::ranges::size(data))));
    ufps::ensure(compressed_data);

    const auto data_path = output_blobs_dir / file_name;
    auto data_file = std::ofstream{data_path, std::ios::binary};
    data_file.write(reinterpret_cast<const char *>(compressed_data->data()), compressed_data->size());

    ufps::log::info(
        "wrote {}, data: {} objects: {} bytes, compression ratio: {:.2f}%",
        data_path,
        compressed_data->size(),
        compressed_data->size() * sizeof(T),
        100.0f * compressed_data->size() / (data.size() * sizeof(T)));
}

}

auto main(int argc, char **argv) -> int
{
    try
    {
        ufps::ensure(argc > 2, "usage: ./resource_packer output_dir dir1 dir2...");

        const auto resource_dirs = std::views::iota(2, argc) |
                                   std::views::transform([&](int i) { return std::filesystem::path{argv[i]}; }) |
                                   std::ranges::to<std::vector>();

        const auto output_asset_dir = std::filesystem::path{argv[1]} / "build_assets";
        std::filesystem::create_directories(output_asset_dir);

        const auto output_blobs_dir = output_asset_dir / "blobs";
        std::filesystem::create_directories(output_blobs_dir);

        const auto output_configs_dir = output_asset_dir / "configs";
        std::filesystem::create_directories(output_configs_dir);

        ufps::log::info("packing resources fbx files");

        auto resource_loader = ufps::FileResourceLoader{resource_dirs};
        auto models =
            resource_loader.resources("models") | std::views::filter([](const auto &e) { return e.ends_with(".fbx"); });

        const auto model_textures_map = resource_loader.load_string("configs\\model_textures.yaml");
        const auto model_textures = ufps::yaml::deserialise<ModelTextures>(model_textures_map);
        ufps::ensure(model_textures);

        auto vertex_offset = 0zu;
        auto index_offset = 0zu;
        auto vertex_data = std::vector<ufps::VertexData>{};
        auto index_data = std::vector<std::uint32_t>{};

        auto skeleton_data = std::vector<ufps::Skeleton::Node>{};

        auto texture_names = std::unordered_set<std::string>{
            "textures\\default_BaseColor.dds",
            "textures\\default_Normal.dds",
            "textures\\default_Metallic.dds",
            "textures\\default_AO.dds",
            "textures\\default_Roughness.dds",
            "textures\\default_Emissive.dds",
            "textures\\bullet_hole1.dds",
            "textures\\bullet_hole2.dds",
            "textures\\bullet_hole3.dds",
            "textures\\bullet_hole4.dds",
            "textures\\spark.dds",
        };

        auto model_manifest = ufps::ModelManifestDescription{};
        auto texture_manifest = ufps::TextureManifestDescription{};
        auto skeleton_manifest = ufps::SkeletonManifestDescription{};

        for (const auto &m : models)
        {
            ufps::log::debug("found model resource: {}", m);
            auto [name, sub_models] =
                load_model(resource_loader.load_data_buffer(m), std::filesystem::path{m}.stem().display_string());

            if (sub_models.empty())
            {
                ufps::log::warn("model {} has no submodels, skipping", name);
                continue;
            }

            for (const auto &[_, skeleton] : sub_models)
            {
                if (skeleton)
                {
                    ufps::log::debug("skeleton {}", skeleton->name());
                    const auto nodes = skeleton->nodes();
                    skeleton_manifest.skeletons[skeleton->name()] = ufps::SkeletonManifest{
                        .offset = std::ranges::size(skeleton_data),
                        .size = std::ranges::size(nodes),
                    };
                    skeleton_data.append_range(nodes);
                }
            }

            model_manifest.models[name] =
                std::views::enumerate(sub_models) |
                std::views::transform(
                    [&](const auto &e)
                    {
                        const auto &[index, model] = e;

                        const auto &mesh_data = model.mesh_data;
                        const auto vertex_count = mesh_data.vertices.size();
                        const auto index_count = mesh_data.indices.size();

                        const auto &textures = model_textures->models.find(name);

                        auto albedo_name = "textures\\default_BaseColor.dds"s;
                        auto normal_name = "textures\\default_Normal.dds"s;
                        auto specular_name = "textures\\default_Metallic.dds"s;
                        auto ao_name = "textures\\default_AO.dds"s;
                        auto glossiness_name = "textures\\default_Roughness.dds"s;
                        auto emissive_name = "textures\\default_Emissive.dds"s;

                        if (textures != std::ranges::cend(model_textures->models))
                        {
                            ufps::log::debug("{}", index);
                            albedo_name = textures->second[index].albedo_texture;
                            normal_name = textures->second[index].normal_texture;
                            specular_name = textures->second[index].specular_texture;
                            ao_name = textures->second[index].ao_texture;
                            glossiness_name = textures->second[index].glossiness_texture;
                            emissive_name = textures->second[index].emissive_texture;
                        }

                        texture_names.insert(albedo_name);
                        texture_names.insert(normal_name);
                        texture_names.insert(specular_name);
                        texture_names.insert(ao_name);
                        texture_names.insert(glossiness_name);
                        texture_names.insert(emissive_name);

                        const auto res = ufps::ModelManifest{
                            .mesh_view =
                                {
                                    .index_offset = static_cast<std::uint32_t>(index_offset),
                                    .index_count = static_cast<std::uint32_t>(mesh_data.indices.size()),
                                    .vertex_offset = static_cast<std::uint32_t>(vertex_offset),
                                    .vertex_count = static_cast<std::uint32_t>(mesh_data.vertices.size()),
                                },
                            .albedo_texture = albedo_name,
                            .normal_texture = normal_name,
                            .specular_texture = specular_name,
                            .ao_texture = ao_name,
                            .glossiness_texture = glossiness_name,
                            .emissive_texture = emissive_name,
                        };

                        vertex_data.append_range(mesh_data.vertices);
                        index_data.append_range(mesh_data.indices);

                        vertex_offset += vertex_count;
                        index_offset += index_count;

                        return res;
                    }) |
                std::ranges::to<std::vector>();
        }

        const auto model_manifest_path = output_configs_dir / "model_manifest.yaml";
        auto model_manifest_file = std::ofstream{model_manifest_path};
        model_manifest_file << *ufps::yaml::serialise(model_manifest);

        const auto skeleton_manifest_path = output_configs_dir / "skeleton_manifest.yaml";
        auto skeleton_manifest_file = std::ofstream{skeleton_manifest_path};
        skeleton_manifest_file << *ufps::yaml::serialise(skeleton_manifest);

        ufps::log::info("finished packing models, packing textures");
        auto texture_blob = ufps::DataBuffer{};

        auto offset = 0zu;

        for (const auto &t : texture_names)
        {
            ufps::log::debug("packing texture: {}", t);

            try
            {
                const auto texture_data = resource_loader.load_data_buffer(t);
                const auto size = texture_data.size();

                texture_blob.append_range(std::as_bytes(std::span{texture_data.data(), size}));

                texture_manifest.textures[t] = {
                    .offset = static_cast<std::uint32_t>(offset),
                    .size = static_cast<std::uint32_t>(size),
                    .is_srgb = t.contains("BaseColor"),
                };

                offset += size;
            }
            catch (ufps::Exception &e)
            {
                ufps::log::warn("missing texture: {}", t);
            }
        }

        const auto texture_manifest_path = output_configs_dir / "texture_manifest.yaml";
        auto texture_manifest_file = std::ofstream{texture_manifest_path};
        texture_manifest_file << *ufps::yaml::serialise(texture_manifest);

        ufps::log::info("finished packing textures, writing to disk");

        write_blob(vertex_data, output_blobs_dir, "vertex_data.bin");
        write_blob(index_data, output_blobs_dir, "index_data.bin");
        write_blob(texture_blob, output_blobs_dir, "texture_data.bin");
        write_blob(skeleton_data, output_blobs_dir, "skeleton_data.bin");
    }
    catch (const ufps::Exception &e)
    {
        ufps::log::error("error packing resources: {}", e.what());
        return 1;
    }
    catch (...)
    {
        ufps::log::error("enknown error packing resources");
        return 1;
    }
}
