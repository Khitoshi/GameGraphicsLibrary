#include "FbxImporter.h"
#include <mutex>
#include "Geometry.h"

namespace Content
{
    namespace {
        std::mutex fbx_mutex{};
    }

    bool FBXContext::InitializeFBX()
    {
        assert(!IsValid());

        fbx_manager_ = FbxManager::Create();
        if (!fbx_manager_)return false;

        FbxIOSettings* ios{ FbxIOSettings::Create(fbx_manager_, IOSROOT) };
        assert(ios);
        fbx_manager_->SetIOSettings(ios);

        return true;
    }

    void FBXContext::LoadFBXFile(const char* file)
    {
        assert(fbx_manager_);
        assert(!fbx_scene_);

        fbx_scene_ = FbxScene::Create(fbx_manager_, "Importer Scene");
        if (!fbx_scene_)return;

        FbxImporter* importer{ FbxImporter::Create(fbx_manager_, "Importer") };
        if (!(importer &&
            importer->Initialize(file, -1, fbx_manager_->GetIOSettings()) &&
            importer->Import(fbx_scene_)))
        {
            return;
        }

        importer->Destroy();

        // Get scene scale in meters.
        scene_scale_ = (f32)fbx_scene_->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(FbxSystemUnit::m);
    }

    void FBXContext::GetScene(FbxNode* root)
    {
        assert(IsValid());

        if (!root) {
            root = fbx_scene_->GetRootNode();
            if (!root) return;
        }

        const s32 num_nodes{ root->GetChildCount() };

        if (scene_data_->settings.coalesce_meshes) {
            LODGroup lod{};
            for (s32 i{ 0 }; i < num_nodes; ++i)
            {
                FbxNode* node{ root->GetChild(i) };
                if (!node) continue;

                GetMeshes(node, lod.meshes, 0, -1.f);
            }

            if (lod.meshes.size()) {
                lod.name = lod.meshes[0].name;
                Mesh combined_mesh{};

                if (CoalesceMeshes(lod, combined_mesh, progression_)) {
                    lod.meshes.clear();
                    lod.meshes.emplace_back(combined_mesh);
                }
                scene_->lod_groups.emplace_back(lod);
            }
        }
        else {
            for (s32 i{ 0 }; i < num_nodes; ++i)
            {
                FbxNode* node{ root->GetChild(i) };
                if (!node) continue;

                LODGroup lod{};
                GetMeshes(node, lod.meshes, 0, -1.f);
                if (lod.meshes.size()) {
                    lod.name = lod.meshes[0].name;
                    scene_->lod_groups.emplace_back(lod);
                }
            }
        }
    }

    void FBXContext::GetMeshes(FbxNode* node, Utility::vector<Mesh>& meshes, u32 lod_id, f32 lod_threshold)
    {
        assert(node && lod_id != u32_invalid_id);
        bool is_lod_group{ false };

        if (const s32 num_attributes{ node->GetNodeAttributeCount() }) {
            for (s32 i{ 0 }; i < num_attributes; ++i)
            {
                FbxNodeAttribute* attribute{ node->GetNodeAttributeByIndex(i) };
                const FbxNodeAttribute::EType attribute_type{ attribute->GetAttributeType() };
                if (attribute_type == FbxNodeAttribute::eMesh) {
                    GetMesh(attribute, meshes, lod_id, lod_threshold);
                }
                else if (attribute_type == FbxNodeAttribute::eLODGroup) {
                    GetLodGroup(attribute);
                    is_lod_group = true;
                }
            }
        }

        if (!is_lod_group) {
            if (const s32 num_children{ node->GetChildCount() }) {
                for (s32 i{ 0 }; i < num_children; ++i)
                {
                    GetMeshes(node->GetChild(i), meshes, lod_id, lod_threshold);
                }
            }
        }
    }

    void FBXContext::GetMesh(FbxNodeAttribute* attribute, Utility::vector<Mesh>& meshes, u32 lod_id, f32 lod_threshold)
    {
        assert(attribute);

        FbxMesh* fbx_mesh{ (FbxMesh*)attribute };
        if (fbx_mesh->RemoveBadPolygons() < 0) return;

        // 必要であればメッシュを三角測量する。
        FbxGeometryConverter gc{ fbx_manager_ };
        fbx_mesh = (FbxMesh*)gc.Triangulate(fbx_mesh, true);
        if (!fbx_mesh || fbx_mesh->RemoveBadPolygons() < 0) return;

        FbxNode* const node{ fbx_mesh->GetNode() };

        Mesh m;
        m.lod_id = lod_id;
        m.lod_threshold = lod_threshold;
        m.name = (node->GetName()[0] != '\0') ? node->GetName() : fbx_mesh->GetName();

        if (GetMeshData(fbx_mesh, m)) {
            meshes.emplace_back(m);
            progression_->Callback(progression_->value(), progression_->max_value() + 1);
        }
    }

    void FBXContext::GetLodGroup(FbxNodeAttribute* attribute)
    {
        assert(attribute);

        FbxLODGroup* lod_grp{ (FbxLODGroup*)attribute };
        FbxNode* const node{ lod_grp->GetNode() };
        LODGroup lod{};
        lod.name = (node->GetName()[0] != '\0') ? node->GetName() : lod_grp->GetName();
        // NOTE: LODの数はベースメッシュ（LOD 0）専用です。
        const s32 num_nodes{ node->GetChildCount() };
        assert(num_nodes > 0 && lod_grp->GetNumThresholds() == (num_nodes - 1));

        for (s32 i{ 0 }; i < num_nodes; ++i)
        {
            f32 lod_threshold{ -1.f };
            if (i > 0) {
                FbxDistance threshold;
                lod_grp->GetThreshold(i - 1, threshold);
                lod_threshold = threshold.value() * scene_scale_;
            }

            GetMeshes(node->GetChild(i), lod.meshes, (u32)lod.meshes.size(), lod_threshold);
        }

        if (lod.meshes.size()) scene_->lod_groups.emplace_back(lod);
    }

    bool FBXContext::GetMeshData(FbxMesh* fbx_mesh, Mesh& m)
    {
        assert(fbx_mesh);

        FbxNode* const node{ fbx_mesh->GetNode() };
        FbxAMatrix geometricTransform;

        geometricTransform.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
        geometricTransform.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
        geometricTransform.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));

        FbxAMatrix transform{ node->EvaluateGlobalTransform() * geometricTransform };
        FbxAMatrix inverse_transpose{ transform.Inverse().Transpose() };

        const s32 num_polys{ fbx_mesh->GetPolygonCount() };
        if (num_polys <= 0) return false;

        // Get vertices
        const s32 num_vertices{ fbx_mesh->GetControlPointsCount() };
        FbxVector4* vertices{ fbx_mesh->GetControlPoints() };
        const s32 num_indices{ fbx_mesh->GetPolygonVertexCount() };
        s32* indices{ fbx_mesh->GetPolygonVertices() };

        assert(num_vertices > 0 && vertices && num_indices > 0 && indices);
        if (!(num_vertices > 0 && vertices && num_indices > 0 && indices)) return false;

        m.raw_indices.resize(num_indices);
        Utility::vector vertex_ref(num_vertices, u32_invalid_id);

        for (s32 i{ 0 }; i < num_indices; ++i)
        {
            const u32 v_idx{ (u32)indices[i] };
            // この頂点に以前出会ったことがあるか？
            // そうでなければ、頂点と新しいインデックスを追加する。
            if (vertex_ref[v_idx] != u32_invalid_id) {
                m.raw_indices[i] = vertex_ref[v_idx];
            }
            else {
                FbxVector4 v = transform.MultT(vertices[v_idx]) * scene_scale_;
                m.raw_indices[i] = (u32)m.positions.size();
                vertex_ref[v_idx] = m.raw_indices[i];
                m.positions.emplace_back((f32)v[0], (f32)v[1], (f32)v[2]);
            }
        }

        assert(m.raw_indices.size() % 3 == 0);

        // ポリゴンごとの材料インデックスを取得
        assert(num_polys > 0);
        FbxLayerElementArrayTemplate<s32>* mtl_indices;
        if (fbx_mesh->GetMaterialIndices(&mtl_indices)) {
            for (s32 i{ 0 }; i < num_polys; ++i) {
                const s32 mtl_index{ mtl_indices->GetAt(i) };
                assert(mtl_index >= 0);
                m.material_indices.emplace_back((u32)mtl_index);
                if (std::find(m.material_used.begin(), m.material_used.end(), (u32)mtl_index) == m.material_used.end())
                {
                    m.material_used.emplace_back((u32)mtl_index);
                }
            }
        }

        // 法線のインポートはデフォルトでオン
        const bool import_normals{ !scene_data_->settings.calculate_normals };
        // 接線のインポートはデフォルトでオフ
        const bool import_tangents{ !scene_data_->settings.calculate_tangents };

        // 法線のインポート
        if (import_normals) {
            FbxArray<FbxVector4> normals;
            // FBXの組み込みメソッドを使用して法線を計算しますが、法線データがすでに存在しない場合に限ります。
            if (fbx_mesh->GenerateNormals() && fbx_mesh->GetPolygonVertexNormals(normals) && normals.Size() > 0) {
                const s32 num_normals{ normals.Size() };
                for (s32 i{ 0 }; i < num_normals; ++i)
                {
                    FbxVector4 n{ inverse_transpose.MultT(normals[i]) };
                    n.Normalize();
                    m.normals.emplace_back((f32)n[0], (f32)n[1], (f32)n[2]);
                }
            }
            else {
                // FBXから法線をインポートする際に何か問題が発生した。 
                // 独自の法線計算方法に戻す。
                scene_data_->settings.calculate_normals = true;
            }
        }

        // Import tangents
        if (import_tangents) {
            FbxLayerElementArrayTemplate<FbxVector4>* tangents{ nullptr };
            // FBXの組み込みメソッドを使用してタンジェントを計算しますが、
            // タンジェントデータがすでに存在しない場合に限ります。
            if (fbx_mesh->GenerateTangentsData() &&
                fbx_mesh->GetTangents(&tangents) &&
                tangents && tangents->GetCount() > 0) {
                const s32 num_tangent{ tangents->GetCount() };
                for (s32 i{ 0 }; i < num_tangent; ++i)
                {
                    // TODO: この変換が正しいかどうかはわからない...
                    FbxVector4 t{ tangents->GetAt(i) };
                    const f32 handedness{ (f32)t[3] };
                    t[3] = 0.0;
                    t = transform.MultT(t);
                    t.Normalize();
                    m.tangents.emplace_back((f32)t[0], (f32)t[1], (f32)t[2], handedness);
                }
            }
            else {
                // FBXからタンジェントをインポートする際に何か問題が発生した。 
                // 独自のタンジェント計算方法に戻す。
                scene_data_->settings.calculate_tangents = true;
            }
        }

        // Get UVs
        FbxStringList uv_names;
        fbx_mesh->GetUVSetNames(uv_names);
        const s32 uv_set_count{ uv_names.GetCount() };
        // NOTE: uvセットがなくても大丈夫です。
        // 例えば、発光するオブジェクトの中には、uvマップを必要としないものがある。
        m.uv_sets.resize(uv_set_count);

        for (s32 i{ 0 }; i < uv_set_count; ++i) {
            FbxArray<FbxVector2> uvs;
            if (fbx_mesh->GetPolygonVertexUVs(uv_names.GetStringAt(i), uvs)) {
                const s32 num_uvs{ uvs.Size() };
                for (s32 j{ 0 }; j < num_uvs; ++j)
                {
                    m.uv_sets[i].emplace_back((f32)uvs[j][0], (f32)uvs[j][1]);
                }
            }
        }

        return true;
    }

}

