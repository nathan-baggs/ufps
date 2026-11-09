#include "assimp_loader.h"

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <span>
#include <string>
#include <tuple>
#include <vector>

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/Logger.hpp>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/vector3.h>

#include "graphics/mesh_data.h"
#include "graphics/utils.h"
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

}

auto load_model(ufps::DataBufferView model_data) -> std::tuple<std::string, std::vector<ModelData>>
{
    [[maybe_unused]] static auto *logger = []
    {
        ::Assimp::DefaultLogger::create("", ::Assimp::Logger::VERBOSE);
        auto *logger = ::Assimp::DefaultLogger::get();

        logger->attachStream(new SimpleAssimpLogStream<ufps::log::Level::ERR>{}, ::Assimp::Logger::Err);
        logger->attachStream(new SimpleAssimpLogStream<ufps::log::Level::WARN>{}, ::Assimp::Logger::Warn);

        return logger;
    }();

    auto importer = ::Assimp::Importer{};
    const auto *scene = importer.ReadFileFromMemory(
        model_data.data(),
        model_data.size(),
        ::aiProcess_Triangulate | ::aiProcess_FlipUVs | ::aiProcess_CalcTangentSpace,
        ".fbx");
    contract_assert(scene != nullptr);

    const auto loaded_meshes = std::span<::aiMesh *>(scene->mMeshes, scene->mMeshes + scene->mNumMeshes);
    const auto materials = std::span<::aiMaterial *>(scene->mMaterials, scene->mMaterials + scene->mNumMaterials);
    ufps::log::info("found {} meshes, {} materials", std::ranges::size(loaded_meshes), std::ranges::size(materials));

    auto models = std::vector<ModelData>{};

    for (const auto &[index, mesh] : loaded_meshes | std::views::enumerate)
    {
        ufps::log::info("found mesh: {}", mesh->mName.C_Str());

        if (mesh->mMaterialIndex >= scene->mNumMaterials)
        {
            ufps::log::warn("mesh {} has invalid material index: {}", mesh->mName.C_Str(), index);
            continue;
        }

        const auto *material = scene->mMaterials[mesh->mMaterialIndex];

        for (auto i = 0; i <= ::aiTextureType_GLTF_METALLIC_ROUGHNESS; ++i)
        {
            const auto count = material->GetTextureCount(static_cast<::aiTextureType>(i));
            if (count >= 1)
            {
                ufps::log::debug("material {} has {} textures of type {}", material->GetName().C_Str(), count, i);
            }
        }

        const auto base_colour_count = material->GetTextureCount(::aiTextureType_BASE_COLOR);
        const auto diffuse_colour_count = material->GetTextureCount(::aiTextureType_DIFFUSE);
        if (base_colour_count != 1 && diffuse_colour_count != 1)
        {
            ufps::log::warn("unsupported base colour count: {} {}", base_colour_count, diffuse_colour_count);
            continue;
        }

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
            std::views::transform([](const auto &v) { return ufps::UV{.s = v.x, .t = v.y}; });

        auto indices =
            std::span<::aiFace>{mesh->mFaces, mesh->mFaces + mesh->mNumFaces} |
            std::views::transform([](const auto &e)
                                  { return std::span<std::uint32_t>{e.mIndices, e.mIndices + e.mNumIndices}; }) |
            std::views::join | std::ranges::to<std::vector>();

        models.push_back({
            .mesh_data =
                {
                    .vertices = ufps::vertices(positions, normals, tangents, bitangents, uvs),
                    .indices = std::move(indices),
                },
        });
    }

    return {loaded_meshes.front()->mName.C_Str(), models};
}
