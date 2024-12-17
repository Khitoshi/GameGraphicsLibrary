#pragma once
#include "CommonHeaders.h"
#include <fbxsdk.h>
#include <Windows.h>
#include "ContentCommon.h"

namespace Content {

    struct SceneData;
    struct Scene;
    struct Mesh;
    struct GeometryImportSettings;

    class FBXContext {
    public:
        FBXContext(const char* file, Scene* scene, SceneData* data, Progression* const progression) :
            scene_{ scene },
            scene_data_{ data },
            progression_{ progression }
        {
            assert(file && scene_ && scene_data_ && progression_);
            if (InitializeFBX()) {
                LoadFBXFile(file);
                assert(IsValid());
            }
        }

        ~FBXContext()
        {
            fbx_scene_->Destroy();
            fbx_manager_->Destroy();
            ZeroMemory(this, sizeof(FBXContext));
        }

        void GetScene(FbxNode* root = nullptr);

        constexpr bool IsValid() const { return fbx_manager_ && fbx_scene_; }
        constexpr f32 SceneScale() const { return scene_scale_; }
        constexpr Progression* GetProgression() const { return progression_; }
    private:

        bool InitializeFBX();
        void LoadFBXFile(const char* file);
        void GetMeshes(FbxNode* node, Utility::vector<Mesh>& meshes, u32 lod_id, f32 lod_threshold);
        void GetMesh(FbxNodeAttribute* attribute, Utility::vector<Mesh>& meshes, u32 lod_id, f32 lod_threshold);
        void GetLodGroup(FbxNodeAttribute* attribute);
        bool GetMeshData(FbxMesh* fbx_mesh, Mesh& m);

        Scene* scene_{ nullptr };
        SceneData* scene_data_{ nullptr };
        FbxManager* fbx_manager_{ nullptr };
        FbxScene* fbx_scene_{ nullptr };
        Progression* progression_{ nullptr };
        f32 scene_scale_{ 1.0f };
    };

}