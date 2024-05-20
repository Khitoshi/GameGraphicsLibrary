#pragma once

#include "ContentCommon.h"
#include <string>

namespace Content {

    struct Vertex
    {
        Math::v4    tangent{};
        Math::v4    joint_weights{};
        Math::u32v4 joint_indices{ u32_invalid_id, u32_invalid_id , u32_invalid_id , u32_invalid_id };
        Math::v3    position{};
        Math::v3    normal{};
        Math::v2    uv{};
        u8          red{}, green{}, blue{};
        u8          pad;
    };

    namespace Elements {

        struct ElementsType {
            enum TYPE : u32 {
                POSITION_ONLY = 0x00,
                STATIC_NORMAL = 0x01,
                STATIC_NORMAL_TEXTURE = 0x03,
                STATIC_COLOR = 0x04,
                SKELETAL = 0x08,
                SKELETAL_COLOR = SKELETAL | STATIC_COLOR,
                SKELETAL_NORMAL = SKELETAL | STATIC_NORMAL,
                SKELETAL_NORMAL_COLOR = SKELETAL_NORMAL | STATIC_COLOR,
                SKELETAL_NORMAL_TEXTURE = SKELETAL | STATIC_NORMAL_TEXTURE,
                SKELETAL_NORMAL_TEXTURE_COLOR = SKELETAL_NORMAL_TEXTURE | STATIC_COLOR,
            };
        };

        struct StaticColor
        {
            u8          color[3];
            u8          pad;
        };

        struct StaticNormal
        {
            u8          color[3];
            u8          t_sign;     // bit 0: tangent handedness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1).
            u16         normal[2];
        };

        struct StaticNormalTexture
        {
            u8          color[3];
            u8          t_sign;     // bit 0: tangent handedness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1).
            u16         normal[2];
            u16         tangent[2];
            Math::v2    uv;
        };

        struct Skeletal
        {
            u8          joint_weights[3]; // normalized joint weights for up to 4 joints.
            u8          pad;
            u16         joint_indices[4];
        };

        struct SkeletalColor
        {
            u8          joint_weights[3]; // normalized joint weights for up to 4 joints.
            u8          pad;
            u16         joint_indices[4];
            u8          color[3];
            u8          pad2;
        };

        struct SkeletalNormal
        {
            u8          joint_weights[3];   // normalized joint weights for up to 4 joints.
            u8          t_sign;             // bit 0: tangent handedness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1).
            u16         joint_indices[4];
            u16         normal[2];
        };

        struct SkeletalNormalColor
        {
            u8          joint_weights[3];   // normalized joint weights for up to 4 joints.
            u8          t_sign;             // bit 0: tangent handedness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1).
            u16         joint_indices[4];
            u16         normal[2];
            u8          color[3];
            u8          pad;
        };

        struct SkeletalNormalTexture
        {
            u8          joint_weights[3];   // normalized joint weights for up to 4 joints.
            u8          t_sign;             // bit 0: tangent handedness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1).
            u16         joint_indices[4];
            u16         normal[2];
            u16         tangent[2];
            Math::v2    uv;
        };

        struct SkeletalNormalTextureColor
        {
            u8          joint_weights[3];   // normalized joint weights for up to 4 joints.
            u8          t_sign;             // bit 0: tangent handedness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1).
            u16         joint_indices[4];
            u16         normal[2];
            u16         tangent[2];
            Math::v2    uv;
            u8          color[3];
            u8          pad;
        };

    } // namespace elements

    struct Mesh
    {
        // Initial data
        Utility::vector<Math::v3>               positions;
        Utility::vector<Math::v3>               normals;
        Utility::vector<Math::v4>               tangents;
        Utility::vector<Math::v3>               colors;
        Utility::vector<Utility::vector<Math::v2>>  uv_sets;
        Utility::vector<u32>                    material_indices;
        Utility::vector<u32>                    material_used;

        Utility::vector<u32>                    raw_indices;

        // Intermediate data
        Utility::vector<Vertex>                 vertices;
        Utility::vector<u32>                    indices;

        // Output data
        std::string                         name;
        Elements::ElementsType::TYPE       elements_type;
        Utility::vector<u8>                     position_buffer;
        Utility::vector<u8>                     element_buffer;

        f32                                 lod_threshold{ -1.f };
        u32                                 lod_id{ u32_invalid_id };
    };

    struct LODGroup
    {
        std::string         name;
        Utility::vector<Mesh>   meshes;
    };

    struct Scene
    {
        std::string             name;
        Utility::vector<LODGroup>  lod_groups;
    };

    struct GeometryImportSettings
    {
        f32 smoothing_angle;
        u8  calculate_normals;
        u8  calculate_tangents;
        u8  reverse_handedness;
        u8  import_embedded_textures;
        u8  import_animations;
        u8  coalesce_meshes;
    };

    struct SceneData
    {
        u8* buffer;
        u32                      buffer_size;
        GeometryImportSettings settings;
    };

    void ProcessScene(Scene& scene, const GeometryImportSettings& settings, Progression* const progression);
    void PackData(const Scene& scene, SceneData& data);
    bool CoalesceMeshes(const LODGroup& lod, Mesh& combined_mesh, Progression* const progression);
}
