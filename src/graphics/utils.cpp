#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/Logger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "assimp/material.h"
#include "assimp/mesh.h"
#include "assimp/vector3.h"
#include "graphics/mesh_data.h"
#include "graphics/model_data.h"
#include "graphics/texture_data.h"
#include "graphics/utils.h"
#include "resources/resource_loader.h"
#include "utils/data_buffer.h"

#include "utils/error.h"
#include "utils/log.h"

namespace
{

template <ufps::log::Level L>
class SimpleAssimpLogStream : public ::Assimp::LogStream
{
  public:
    auto write(const char *msg) -> void override
    {
        if constexpr (L == ufps::log::Level::INFO)
        {
            ufps::log::info("{}", msg);
        }
        else if constexpr (L == ufps::log::Level::WARN)
        {
            ufps::log::warn("{}", msg);
        }
        else if constexpr (L == ufps::log::Level::ERR)
        {
            ufps::log::error("{}", msg);
        }
        else if constexpr (L == ufps::log::Level::DEBUG)
        {
            ufps::log::debug("{}", msg);
        }
        else
        {
            ufps::log::error("[unknown level] {}", msg);
        }
    }
};

auto to_native(const ::aiVector3D &v) -> ufps::Vector3
{
    return {v.x, v.y, v.z};
}

auto channels_to_format(int num_channels) -> ufps::TextureFormat
{
    switch (num_channels)
    {
        using enum ufps::TextureFormat;

        case 1: return RED;
        case 3: return RGB;
        case 4: return RGBA;
    }

    throw ufps::Exception("unsupported channel count: {}", num_channels);
}

}

namespace ufps
{
auto load_texture(DataBufferView image_data) -> TextureData
{
    auto width = int{};
    auto height = int{};
    auto num_channels = int{};

    ::stbi_set_flip_vertically_on_load(true);
    auto raw_data = std::unique_ptr<::stbi_uc, void (*)(void *)>{
        ::stbi_load_from_memory(
            reinterpret_cast<const ::stbi_uc *>(image_data.data()),
            image_data.size(),
            &width,
            &height,
            &num_channels,
            0),
        ::stbi_image_free};
    ensure(raw_data, "failed to parse texture data");

    const auto *ptr = reinterpret_cast<const std::byte *>(raw_data.get());

    return {
        .width = static_cast<std::uint32_t>(width),
        .height = static_cast<std::uint32_t>(height),
        .format = channels_to_format(num_channels),
        .data = {{ptr, ptr + width * height * num_channels}}};
}

auto load_model(DataBufferView model_data, ResourceLoader &resource_loader) -> std::vector<ModelData>
{
    [[maybe_unused]] static auto *logger = []
    {
        ::Assimp::DefaultLogger::create("", ::Assimp::Logger::VERBOSE);
        auto *logger = ::Assimp::DefaultLogger::get();

        logger->attachStream(new SimpleAssimpLogStream<ufps::log::Level::ERR>{}, ::Assimp::Logger::Err);
        logger->attachStream(new SimpleAssimpLogStream<ufps::log::Level::DEBUG>{}, ::Assimp::Logger::Debugging);
        logger->attachStream(new SimpleAssimpLogStream<ufps::log::Level::WARN>{}, ::Assimp::Logger::Warn);
        logger->attachStream(new SimpleAssimpLogStream<ufps::log::Level::INFO>{}, ::Assimp::Logger::Info);

        return logger;
    }();

    auto importer = ::Assimp::Importer{};
    const auto *scene = importer.ReadFileFromMemory(
        model_data.data(),
        model_data.size(),
        ::aiProcess_Triangulate | ::aiProcess_FlipUVs | ::aiProcess_CalcTangentSpace,
        ".fbx");
    ensure(scene != nullptr, "failed to parse assimp scene");

    const auto loaded_meshes = std::span<::aiMesh *>(scene->mMeshes, scene->mMeshes + scene->mNumMeshes);
    const auto materials = std::span<::aiMaterial *>(scene->mMaterials, scene->mMaterials + scene->mNumMaterials);
    log::info("found {} meshes, {} materials", std::ranges::size(loaded_meshes), std::ranges::size(materials));

    ensure(
        std::ranges::size(loaded_meshes) == std::ranges::size(materials), "mismatch mesh/material count in model file");

    auto models = std::vector<ModelData>{};

    for (const auto &[index, mesh] : loaded_meshes | std::views::enumerate)
    {
        log::info("found mesh: {}", mesh->mName.C_Str());

        const auto *material = scene->mMaterials[index];
        const auto base_colour_count = material->GetTextureCount(::aiTextureType_BASE_COLOR);
        if (base_colour_count != 1)
        {
            log::warn("unsupported base colour count: {}", base_colour_count);
            continue;
        }

        auto path_str = ::aiString{};
        material->GetTexture(::aiTextureType_BASE_COLOR, 0u, &path_str);
        const auto path = std::filesystem::path{path_str.C_Str()};
        const auto filename = path.filename();
        log::info("found base colour texture: {}", filename.string());

        const auto positions = std::span<::aiVector3D>{mesh->mVertices, mesh->mVertices + mesh->mNumVertices} |
                               std::views::transform(to_native);
        const auto normals = std::span<::aiVector3D>{mesh->mNormals, mesh->mNormals + mesh->mNumVertices} |
                             std::views::transform(to_native);
        const auto tangents = std::span<::aiVector3D>{mesh->mTangents, mesh->mTangents + mesh->mNumVertices} |
                              std::views::transform(to_native);
        const auto bitangents = std::span<::aiVector3D>{mesh->mBitangents, mesh->mBitangents + mesh->mNumVertices} |
                                std::views::transform(to_native);
        const auto uvs =
            std::span<::aiVector3D>{mesh->mTextureCoords[0], mesh->mTextureCoords[0] + mesh->mNumVertices} |
            std::views::transform([](const auto &v) { return UV{.s = v.x, .t = v.y}; });

        auto indices =
            std::span<::aiFace>{mesh->mFaces, mesh->mFaces + mesh->mNumFaces} |
            std::views::transform([](const auto &e)
                                  { return std::span<std::uint32_t>{e.mIndices, e.mIndices + e.mNumIndices}; }) |
            std::views::join | std::ranges::to<std::vector>();

        models.push_back({
            .mesh_data =
                MeshData{
                    .vertices = vertices(positions, normals, tangents, bitangents, uvs),
                    .indices = std::move(indices),
                },
            .albedo = load_texture(resource_loader.load_data_buffer(std::format("textures\\{}", filename.string()))),
            .normal = std::nullopt,
            .specular = std::nullopt,
        });
    }

    return models;
}
}
