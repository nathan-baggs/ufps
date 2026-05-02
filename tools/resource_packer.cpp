#include <filesystem>
#include <fstream>
#include <ranges>
#include <unordered_set>

#include <yaml-cpp/yaml.h>

#include "core/manifest_descriptions.h"
#include "graphics/utils.h"
#include "resources/file_resource_loader.h"
#include "serialisation/yaml_serialiser.h"
#include "utils/compress.h"
#include "utils/error.h"
#include "utils/log.h"

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

        auto vertex_offset = 0zu;
        auto index_offset = 0zu;
        auto vertex_data = std::vector<ufps::VertexData>{};
        auto index_data = std::vector<std::uint32_t>{};

        auto texture_names = std::unordered_set<std::string>{
            "textures\\default_BaseColor.dds",
            "textures\\default_Normal.dds",
            "textures\\default_Metallic.dds",
        };

        {
            auto manifest = ufps::ModelManifestDescription{};

            for (const auto &m : models)
            {
                ufps::log::debug("found model resource: {}", m);
                const auto &[name, sub_models] = ufps::load_model(resource_loader.load_data_buffer(m));

                if (sub_models.empty())
                {
                    ufps::log::warn("model {} has no submodels, skipping", name);
                    continue;
                }

                manifest.models[name] =
                    sub_models |
                    std::views::transform(
                        [&](const auto &model)
                        {
                            const auto &mesh_data = model.mesh_data;
                            const auto vertex_count = mesh_data.vertices.size();
                            const auto index_count = mesh_data.indices.size();

                            const auto albedo_name = model.albedo ? *model.albedo : "textures\\default_BaseColor.dds";
                            const auto normal_name = model.normal ? *model.normal : "textures\\default_Normal.dds";
                            const auto specular_name =
                                model.specular ? *model.specular : "textures\\default_Metallic.dds";

                            texture_names.insert(albedo_name);
                            texture_names.insert(normal_name);
                            texture_names.insert(specular_name);

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
                            };

                            vertex_data.append_range(mesh_data.vertices);
                            index_data.append_range(mesh_data.indices);

                            vertex_offset += vertex_count;
                            index_offset += index_count;

                            return res;
                        }) |
                    std::ranges::to<std::vector>();
            }

            const auto manifest_path = output_configs_dir / "model_manifest.yaml";
            auto manifest_file = std::ofstream{manifest_path};
            manifest_file << ufps::yaml::serialise(manifest);
        }

        ufps::log::info("finished packing models, packing textures");
        auto texture_blob = ufps::DataBuffer{};

        {
            auto manifest = ufps::TextureManifestDescription{};

            auto offset = 0zu;

            for (const auto &t : texture_names)
            {
                ufps::log::debug("packing texture: {}", t);

                const auto texture_data = resource_loader.load_data_buffer(t);
                const auto size = texture_data.size();

                texture_blob.append_range(std::as_bytes(std::span{texture_data.data(), size}));

                manifest.textures[t] = {
                    .offset = static_cast<std::uint32_t>(offset),
                    .size = static_cast<std::uint32_t>(size),
                    .is_srgb = t.contains("BaseColor"),
                };

                offset += size;
            }

            const auto manifest_path = output_configs_dir / "texture_manifest.yaml";
            auto manifest_file = std::ofstream{manifest_path};

            manifest_file << ufps::yaml::serialise(manifest);
        }

        ufps::log::info("finished packing textures, writing to disk");

        const auto compressed_vertex_data =
            ufps::compress(std::as_bytes(std::span{vertex_data.data(), vertex_data.size()}));

        const auto vertex_data_path = output_blobs_dir / "vertex_data.bin";
        auto vertex_data_file = std::ofstream{vertex_data_path, std::ios::binary};
        vertex_data_file.write(
            reinterpret_cast<const char *>(compressed_vertex_data.data()), compressed_vertex_data.size());

        ufps::log::info(
            "wrote vertex data: {} vertices, {} bytes, compression ratio: {:.2f}%",
            compressed_vertex_data.size(),
            compressed_vertex_data.size() * sizeof(ufps::VertexData),
            100.0f * compressed_vertex_data.size() / (vertex_data.size() * sizeof(ufps::VertexData)));

        const auto compressed_index_data =
            ufps::compress(std::as_bytes(std::span{index_data.data(), index_data.size()}));

        const auto index_data_path = output_blobs_dir / "index_data.bin";
        auto index_data_file = std::ofstream{index_data_path, std::ios::binary};
        index_data_file.write(
            reinterpret_cast<const char *>(compressed_index_data.data()), compressed_index_data.size());

        ufps::log::info(
            "wrote index data: {} indices, {} bytes, compression ratio: {:.2f}%",
            compressed_index_data.size(),
            compressed_index_data.size() * sizeof(std::uint32_t),
            100.0f * compressed_index_data.size() / (index_data.size() * sizeof(std::uint32_t)));

        const auto compressed_texture_blob = ufps::compress(texture_blob);

        const auto texture_blob_path = output_blobs_dir / "texture_data.bin";
        auto texture_blob_file = std::ofstream{texture_blob_path, std::ios::binary};
        texture_blob_file.write(
            reinterpret_cast<const char *>(compressed_texture_blob.data()), compressed_texture_blob.size());

        ufps::log::info(
            "wrote texture blob: {} textures, {} bytes, compression ratio: {:.2f}%",
            texture_names.size(),
            compressed_texture_blob.size(),
            100.0f * compressed_texture_blob.size() / texture_blob.size());
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
