#include <filesystem>
#include <fstream>
#include <ranges>
#include <unordered_set>

#include <yaml-cpp/yaml.h>

#include "graphics/utils.h"
#include "resources/file_resource_loader.h"
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

        auto texture_names = std::unordered_set<std::string>{};

        {
            const auto manifest_path = output_configs_dir / "model_manifest.yaml";
            auto manifest_file = std::ofstream{manifest_path};

            auto out = ::YAML::Emitter{manifest_file};
            out << ::YAML::BeginMap;

            for (const auto &m : models)
            {
                ufps::log::debug("found model resource: {}", m);
                const auto &[name, sub_models] = ufps::load_model(resource_loader.load_data_buffer(m));

                if (sub_models.empty())
                {
                    ufps::log::warn("model {} has no submodels, skipping", name);
                    continue;
                }

                out << ::YAML::Key << name << ::YAML::Value << ::YAML::BeginMap;

                for (const auto &[index, model] : sub_models | std::views::enumerate)
                {
                    const auto &mesh_data = model.mesh_data;
                    const auto vertex_count = mesh_data.vertices.size();
                    const auto index_count = mesh_data.indices.size();

                    ufps::log::debug(
                        "model: {}, submodel: {}, vertex count: {}, index count: {}, vertex offset: {}, index offset: "
                        "{}",
                        name,
                        index,
                        vertex_count,
                        index_count,
                        vertex_offset,
                        index_offset);

                    vertex_data.append_range(mesh_data.vertices);
                    index_data.append_range(mesh_data.indices);

                    out << ::YAML::Key << std::format("submodel_{}", index) << ::YAML::Value << ::YAML::BeginMap;
                    out << ::YAML::Key << "vertex_count" << ::YAML::Value << vertex_count;
                    out << ::YAML::Key << "vertex_offset" << ::YAML::Value << vertex_offset;
                    out << ::YAML::Key << "index_count" << ::YAML::Value << index_count;
                    out << ::YAML::Key << "index_offset" << ::YAML::Value << index_offset;

                    const auto albedo_name = model.albedo ? *model.albedo : "textures\\default_BaseColor.dds";
                    texture_names.insert(albedo_name);
                    out << ::YAML::Key << "albedo_name" << ::YAML::Value << albedo_name;

                    const auto normal_name = model.normal ? *model.normal : "textures\\default_Normal.dds";
                    texture_names.insert(normal_name);
                    out << ::YAML::Key << "normal_name" << ::YAML::Value << normal_name;

                    const auto specular_name = model.specular ? *model.specular : "textures\\default_Metallic.dds";
                    out << ::YAML::Key << "specular_name" << ::YAML::Value << specular_name;

                    out << ::YAML::EndMap;

                    vertex_offset += vertex_count;
                    index_offset += index_count;
                }

                out << ::YAML::EndMap;
            }
        }

        ufps::log::info("finished packing models, packing textures");
        auto texture_blob = ufps::DataBuffer{};

        {
            const auto manifest_path = output_configs_dir / "texture_manifest.yaml";
            auto manifest_file = std::ofstream{manifest_path};

            auto out = ::YAML::Emitter{manifest_file};
            out << ::YAML::BeginMap;

            auto offset = 0zu;

            for (const auto &t : texture_names)
            {
                ufps::log::debug("packing texture: {}", t);

                const auto texture_data = resource_loader.load_data_buffer(t);
                const auto size = texture_data.size();

                texture_blob.append_range(std::as_bytes(std::span{texture_data.data(), size}));

                out << ::YAML::Key << t << ::YAML::Value << ::YAML::BeginMap;
                out << ::YAML::Key << "offset" << ::YAML::Value << offset;
                out << ::YAML::Key << "size" << ::YAML::Value << size;
                out << ::YAML::EndMap;

                offset += size;
            }

            out << ::YAML::EndMap;
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
