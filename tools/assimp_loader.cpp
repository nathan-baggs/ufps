#include "assimp_loader.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
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

#include "animation/bone.h"
#include "animation/skeleton.h"
#include "graphics/mesh_data.h"
#include "graphics/utils.h"
#include "graphics/vertex_data.h"
#include "maths/matrix4.h"
#include "maths/vector3.h"
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
    return std::bit_cast<ufps::Vector3>(v);
}

[[maybe_unused]] auto to_native_mat(const ::aiMatrix4x4 &v) -> ufps::Matrix4
{
    return std::bit_cast<ufps::Matrix4>(v);
}

struct SkeletonBuilder
{
    std::vector<ufps::Skeleton::Node> nodes;
    std::stack<std::size_t> parents;

    ~SkeletonBuilder()
    {
        contract_assert(std::ranges::empty(parents));
    }

    auto push_parent()
    {
        parents.push(std::ranges::size(nodes));
    }

    auto pop_parent()
    {
        contract_assert(!std::ranges::empty(parents));
        parents.pop();
    }

    auto insert(std::string_view name, const ufps::Matrix4 &bind_local, const ufps::Matrix4 &inverse_bind)
    {
        contract_assert(
            (std::ranges::empty(nodes) && std::ranges::empty(parents)) ||
            (!std::ranges::empty(nodes) && !std::ranges::empty(parents)));

        nodes.push_back({
            .name = std::string{name},
            .bind_local = bind_local,
            .inverse_bind = inverse_bind,
            .parent = std::ranges::empty(parents) ? std::nullopt : std::make_optional(parents.top()),
        });
    }
};

auto load_skeleton(const ::aiScene *scene, const ::aiMesh *mesh, std::string_view name)
    -> std::optional<std::tuple<ufps::Skeleton, VertexBoneMap>>
{
    ufps::log::debug("trying to load skeleton for: {}", name);

    auto stack = std::stack<std::tuple<const ::aiNode *, std::optional<std::size_t>>>{};
    stack.emplace(scene->mRootNode, std::nullopt);

    auto nodes = std::vector<ufps::Skeleton::Node>{};

    const auto bone_lookup = std::span{mesh->mBones, mesh->mNumBones};

    auto vertex_bone_map = VertexBoneMap{};

    do
    {
        const auto [node, parent] = stack.top();
        stack.pop();

        auto node_name = std::string_view{node->mName.C_Str()};

        ufps::log::debug("node: {}", node->mName.C_Str());

        auto bone = std::ranges::find_if(
            bone_lookup, [&](const auto *e) { return std::string_view{e->mName.C_Str()} == node_name; });

        const auto index = std::ranges::size(nodes);

        auto inverse_bind = ufps::Matrix4{};
        if (bone != std::ranges::cend(bone_lookup))
        {
            inverse_bind = to_native_mat((*bone)->mOffsetMatrix);

            const auto weights = std::span{(*bone)->mWeights, (*bone)->mNumWeights};
            for (const auto &[vertex_index, weight] : weights)
            {
                vertex_bone_map[vertex_index].emplace_back(index, weight);
            }
        }

        nodes.push_back({
            .name = std::string{name},
            .bind_local = to_native_mat(node->mTransformation),
            .inverse_bind = inverse_bind,
            .parent = parent,
        });

        stack.push_range(
            std::span{node->mChildren, node->mNumChildren} |
            std::views::transform([index](const auto *e) { return std::make_tuple(e, index); }));
    } while (!std::ranges::empty(stack));

    return std::ranges::empty(nodes)
               ? std::nullopt
               : std::make_optional(
                     std::make_tuple(ufps::Skeleton{name, std::move(nodes)}, std::move(vertex_bone_map)));
}
}

auto load_model(ufps::DataBufferView model_data, std::string_view name)
    -> std::tuple<std::string, std::vector<ModelData>>
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
    ufps::log::info(
        "found {} meshes, {} materials, in mesh: {}",
        std::ranges::size(loaded_meshes),
        std::ranges::size(materials),
        name);

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

        auto weights = std::vector<std::array<ufps::Weight, 4zu>>(std::ranges::size(positions));

        auto skeleton = std::optional<ufps::Skeleton>{};
        if (const auto skeleton_data = load_skeleton(scene, mesh, std::format("{}_{}", name, mesh->mName.C_Str()));
            skeleton_data)
        {
            const auto &[loaded_skeleton, vertex_bone_map] = *skeleton_data;
            skeleton = std::move(loaded_skeleton);

            for (const auto &[vertex_index, bone_weights] : vertex_bone_map)
            {
                contract_assert(vertex_index < std::ranges::size(weights));
                std::ranges::copy(bone_weights, std::ranges::begin(weights[vertex_index]));
            }
        }

        models.push_back({
            .mesh_data =
                {
                    .vertices = ufps::vertices(positions, normals, tangents, bitangents, uvs, weights),
                    .indices = std::move(indices),
                },
            .skeleton = std::move(skeleton),
        });
    }

    return {loaded_meshes.front()->mName.C_Str(), models};
}
