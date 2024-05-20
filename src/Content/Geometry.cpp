#include "Geometry.h"
#include "IOStream.h"

#include <Windows.h>

namespace Content
{
    namespace
    {
        using namespace Math;
        using namespace DirectX;

        // 法線の再計算
        void RecalculateNormals(Mesh& m)
        {
            const u32 num_indices{ (u32)m.raw_indices.size() };
            m.normals.resize(num_indices);

            for (u32 i{ 0 }; i < num_indices; ++i) {
                const u32 i0{ m.raw_indices[i] };
                const u32 i1{ m.raw_indices[++i] };
                const u32 i2{ m.raw_indices[++i] };

                // 三角形の頂点を取得
                XMVECTOR v0{ XMLoadFloat3(&m.positions[i0]) };
                XMVECTOR v1{ XMLoadFloat3(&m.positions[i1]) };
                XMVECTOR v2{ XMLoadFloat3(&m.positions[i2]) };

                // 面法線を計算
                XMVECTOR e0{ v1 - v0 };
                XMVECTOR e1{ v2 - v0 };

                // 法線を正規化
                XMVECTOR n{ XMVector3Normalize(XMVector3Cross(e0, e1)) };

                // 同じ三角形を形成する頂点の法線を更新
                XMStoreFloat3(&m.normals[i], n);
                m.normals[i - 1] = m.normals[i];
                m.normals[i - 2] = m.normals[i];
            }
        }

        // 法線の再算出
        void ProcessNormals(Mesh& m, f32 smoothing_angle)
        {
            const f32 cos_alpha{ XMScalarCos(pi - smoothing_angle * pi / 180.f) };

            const bool is_hard_edge{ XMScalarNearEqual(smoothing_angle, 180.f, epsilon) };
            const bool is_soft_edge{ XMScalarNearEqual(smoothing_angle, 0.f, epsilon) };

            const u32 num_indices{ (u32)m.raw_indices.size() };
            const u32 num_vertices{ (u32)m.positions.size() };

            assert(num_indices && num_vertices);

            m.indices.resize(num_indices);

            // 各頂点に対応するインデックスを記録
            Utility::vector<Utility::vector<u32>> idx_ref(num_vertices);
            for (u32 i{ 0 }; i < num_indices; ++i) idx_ref[m.raw_indices[i]].emplace_back(i);

            for (u32 i{ 0 }; i < num_vertices; ++i) {
                auto& refs{ idx_ref[i] };
                u32 num_refs{ (u32)refs.size() };

                for (u32 j{ 0 }; j < num_refs; ++j) {
                    m.indices[refs[j]] = (u32)m.vertices.size();
                    Vertex v{};
                    v.position = m.positions[m.raw_indices[refs[j]]];

                    // 法線ベクトルを読み込む
                    XMVECTOR n1{ XMLoadFloat3(&m.normals[refs[j]]) };
                    if (!is_hard_edge) {// スムージング角度が180度未満の場合
                        for (u32 k{ j + 1 }; k < num_refs; ++k) {
                            // この値は法線間の角度の余弦を表す。
                            f32 cos_theta{ 0.f };
                            XMVECTOR n2{ XMLoadFloat3(&m.normals[refs[k]]) };
                            if (!is_soft_edge) {
                                // NOTE: n1の長さはこのループの繰り返しで変わる可能性があるため、
                                //       この計算ではn1の長さを考慮している。n2の長さは単位とする。 
                                //       cos(angle) = dot(n1, n2) / (||n1||*||n2||)
                                XMStoreFloat(&cos_theta, XMVector3Dot(n1, n2) * XMVector3ReciprocalLength(n1));
                            }

                            // スムージング角度が180度未満の場合、
                            // 法線ベクトルがスムージング角度よりも小さい角度を持つ場合、
                            if (is_soft_edge || cos_theta >= cos_alpha) {
                                n1 += n2;

                                m.indices[refs[k]] = m.indices[refs[j]];
                                refs.erase(refs.begin() + k);
                                --num_refs;
                                --k;
                            }
                        }
                    }
                    // 法線ベクトルを正規化
                    XMStoreFloat3(&v.normal, XMVector3Normalize(n1));
                    m.vertices.push_back(v);
                }
            }
        }

        // UV座標の再計算
        void ProcessUVs(Mesh& m)
        {
            Utility::vector<Vertex> old_vertices;
            old_vertices.swap(m.vertices);
            Utility::vector<u32> old_indices(m.indices.size());
            old_indices.swap(m.indices);

            const u32 num_vertices{ (u32)old_vertices.size() };
            const u32 num_indices{ (u32)old_indices.size() };
            assert(num_vertices && num_indices);

            Utility::vector<Utility::vector<u32>> idx_ref(num_vertices);
            for (u32 i{ 0 }; i < num_indices; ++i) idx_ref[old_indices[i]].emplace_back(i);

            for (u32 i{ 0 }; i < num_vertices; ++i)
            {
                auto& refs{ idx_ref[i] };
                u32 num_refs{ (u32)refs.size() };
                for (u32 j{ 0 }; j < num_refs; ++j)
                {
                    m.indices[refs[j]] = (u32)m.vertices.size();
                    Vertex& v{ old_vertices[old_indices[refs[j]]] };
                    v.uv = m.uv_sets[0][refs[j]];
                    m.vertices.emplace_back(v);

                    for (u32 k{ j + 1 }; k < num_refs; ++k)
                    {
                        // 重複するUV座標を削除
                        v2& uv1{ m.uv_sets[0][refs[k]] };
                        if (XMScalarNearEqual(v.uv.x, uv1.x, epsilon) &&
                            XMScalarNearEqual(v.uv.y, uv1.y, epsilon))
                        {
                            m.indices[refs[k]] = m.indices[refs[j]];
                            refs.erase(refs.begin() + k);
                            --num_refs;
                            --k;
                        }
                    }
                }
            }
        }

        // メッシュの要素のサイズを取得
        u64 GetVertexElementSize(Elements::ElementsType::TYPE elements_type)
        {
            using namespace Elements;
            switch (elements_type)
            {
            case ElementsType::STATIC_NORMAL:                  return sizeof(StaticNormal);
            case ElementsType::STATIC_NORMAL_TEXTURE:          return sizeof(StaticNormalTexture);
            case ElementsType::STATIC_COLOR:                   return sizeof(StaticColor);
            case ElementsType::SKELETAL:                       return sizeof(Skeletal);
            case ElementsType::SKELETAL_COLOR:                 return sizeof(SkeletalColor);
            case ElementsType::SKELETAL_NORMAL:                return sizeof(SkeletalNormal);
            case ElementsType::SKELETAL_NORMAL_COLOR:          return sizeof(SkeletalNormalColor);
            case ElementsType::SKELETAL_NORMAL_TEXTURE:        return sizeof(SkeletalNormalTexture);
            case ElementsType::SKELETAL_NORMAL_TEXTURE_COLOR:  return sizeof(SkeletalNormalTextureColor);
            }

            return 0;
        }

        // 頂点情報のパック
        void PackVertices(Mesh& m)
        {
            const u32 num_vertices{ (u32)m.vertices.size() };
            assert(num_vertices);

            m.position_buffer.resize(sizeof(Math::v3) * num_vertices);
            Math::v3* const position_buffer{ (Math::v3* const)m.position_buffer.data() };

            for (u32 i{ 0 }; i < num_vertices; ++i)
            {
                position_buffer[i] = m.vertices[i].position;
            }

            struct u16v2 { u16 x, y; };
            struct u8v3 { u8 x, y, z; };

            Utility::vector<u8> t_signs(num_vertices);
            Utility::vector<u16v2> normals(num_vertices);
            Utility::vector<u16v2> tangents(num_vertices);
            Utility::vector<u8v3> joint_weights(num_vertices);

            // 法線と接線をパック
            if (m.elements_type & Elements::ElementsType::STATIC_NORMAL) {
                // normals only
                for (u32 i{ 0 }; i < num_vertices; ++i) {
                    Vertex& v{ m.vertices[i] };
                    t_signs[i] = (u8)((v.normal.z > 0.f) << 1);
                    normals[i] = { (u16)PackFloat<16>(v.normal.x, -1.f, 1.f), (u16)PackFloat<16>(v.normal.y, -1.f, 1.f) };
                }

                if (m.elements_type & Elements::ElementsType::STATIC_NORMAL_TEXTURE) {
                    // full T-space
                    for (u32 i{ 0 }; i < num_vertices; ++i)
                    {
                        Vertex& v{ m.vertices[i] };
                        t_signs[i] |= (u8)((v.tangent.w > 0.f) && (v.tangent.z > 0.f));
                        tangents[i] = { (u16)PackFloat<16>(v.tangent.x, -1.f, 1.f), (u16)PackFloat<16>(v.tangent.y, -1.f, 1.f) };
                    }
                }
            }

            // スケルトンアニメーションのためのジョイントウェイトをパック
            if (m.elements_type & Elements::ElementsType::SKELETAL) {
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    // pack joint weights (from [0.0, 1.0] to [0..255])
                    joint_weights[i] = {
                        (u8)PackUnitFloat<8>(v.joint_weights.x),
                        (u8)PackUnitFloat<8>(v.joint_weights.y),
                        (u8)PackUnitFloat<8>(v.joint_weights.z)
                    };

                    // NOTE: w3 will be calculated in shader since joint weights sum to one(1).
                    // NOTE: w3はシェーダーで計算される
                }
            }

            m.element_buffer.resize(GetVertexElementSize(m.elements_type) * num_vertices);
            using namespace Elements;

            // 頂点情報をエレメントバッファに格納
            // insert vertex data into element buffer
            switch (m.elements_type)
            {
            case ElementsType::STATIC_COLOR:
            {
                StaticColor* const element_buffer{ (StaticColor* const)m.element_buffer.data() };
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    element_buffer[i] = { {v.red, v.green, v.blue}, {} };
                }
            }
            break;
            case ElementsType::STATIC_NORMAL:
            {
                StaticNormal* const element_buffer{ (StaticNormal* const)m.element_buffer.data() };
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    element_buffer[i] = { {v.red, v.green, v.blue}, t_signs[i], {normals[i].x, normals[i].y} };
                }
            }
            break;
            case ElementsType::STATIC_NORMAL_TEXTURE:
            {
                StaticNormalTexture* const element_buffer{ (StaticNormalTexture* const)m.element_buffer.data() };
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    element_buffer[i] = { {v.red, v.green, v.blue}, t_signs[i],
                                         {normals[i].x, normals[i].y}, {tangents[i].x, tangents[i].y},
                                         v.uv };
                }
            }
            break;
            case ElementsType::SKELETAL:
            {
                Skeletal* const element_buffer{ (Skeletal* const)m.element_buffer.data() };
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y , (u16)v.joint_indices.z , (u16)v.joint_indices.w };
                    element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, {},
                                         {indices[0], indices[1], indices[2], indices[3]} };
                }
            }
            break;
            case ElementsType::SKELETAL_COLOR:
            {
                SkeletalColor* const element_buffer{ (SkeletalColor* const)m.element_buffer.data() };
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y , (u16)v.joint_indices.z , (u16)v.joint_indices.w };
                    element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, {},
                                         {indices[0], indices[1], indices[2], indices[3]},
                                         {v.red, v.green, v.blue}, {} };
                }
            }
            break;
            case ElementsType::SKELETAL_NORMAL:
            {
                SkeletalNormal* const element_buffer{ (SkeletalNormal* const)m.element_buffer.data() };
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y , (u16)v.joint_indices.z , (u16)v.joint_indices.w };
                    element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, t_signs[i],
                                         {indices[0], indices[1], indices[2], indices[3]},
                                         {normals[i].x, normals[i].y} };
                }
            }
            break;
            case ElementsType::SKELETAL_NORMAL_COLOR:
            {
                SkeletalNormalColor* const element_buffer{ (SkeletalNormalColor* const)m.element_buffer.data() };
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y , (u16)v.joint_indices.z , (u16)v.joint_indices.w };
                    element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, t_signs[i],
                                         {indices[0], indices[1], indices[2], indices[3]},
                                         {normals[i].x, normals[i].y}, {v.red, v.green, v.blue}, {} };
                }
            }
            break;
            case ElementsType::SKELETAL_NORMAL_TEXTURE:
            {
                SkeletalNormalTexture* const element_buffer{ (SkeletalNormalTexture* const)m.element_buffer.data() };
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y , (u16)v.joint_indices.z , (u16)v.joint_indices.w };
                    element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, t_signs[i],
                                         {indices[0], indices[1], indices[2], indices[3]},
                                         {normals[i].x, normals[i].y}, {tangents[i].x, tangents[i].y}, v.uv };
                }
            }
            break;
            case ElementsType::SKELETAL_NORMAL_TEXTURE_COLOR:
            {
                SkeletalNormalTextureColor* const element_buffer{ (SkeletalNormalTextureColor* const)m.element_buffer.data() };
                for (u32 i{ 0 }; i < num_vertices; ++i)
                {
                    Vertex& v{ m.vertices[i] };
                    const u16 indices[4]{ (u16)v.joint_indices.x, (u16)v.joint_indices.y , (u16)v.joint_indices.z , (u16)v.joint_indices.w };
                    element_buffer[i] = { {joint_weights[i].x, joint_weights[i].y, joint_weights[i].z}, t_signs[i],
                                         {indices[0], indices[1], indices[2], indices[3]},
                                         {normals[i].x, normals[i].y}, {tangents[i].x, tangents[i].y}, v.uv,
                                         {v.red, v.green, v.blue}, {} };
                }
            }
            break;
            }
        }

        // メッシュの要素のタイプを決定
        Elements::ElementsType::TYPE DetermineElementsType(const Mesh& m)
        {
            using namespace Elements;
            ElementsType::TYPE type{};

            if (m.normals.size()) {
                if (m.uv_sets.size() && m.uv_sets[0].size()) type = ElementsType::STATIC_NORMAL_TEXTURE;
                else type = ElementsType::STATIC_NORMAL;
            }
            else if (m.colors.size()) {
                type = ElementsType::STATIC_COLOR;
            }

            //TODO: リファクタリング
            //if (m.normals.size() && m.uv_sets.size() && m.uv_sets[0].size())return ElementsType::STATIC_NORMAL_TEXTURE;
            //if (m.normals.size()) return ElementsType::STATIC_NORMAL;
            //if (m.colors.size()) return ElementsType::STATIC_COLOR;

            // TODO: 骨格メッシュのデータがない。骨格メッシュについては後で拡張する。
            return type;
        }

        // 頂点の処理
        void ProcessVertices(Mesh& m, const GeometryImportSettings& settings)
        {
            assert((m.raw_indices.size() % 3) == 0);
            if (settings.calculate_normals || m.normals.empty()) {
                RecalculateNormals(m);
            }

            ProcessNormals(m, settings.smoothing_angle);

            if (!m.uv_sets.empty()) {
                ProcessUVs(m);
            }

            m.elements_type = DetermineElementsType(m);
            PackVertices(m);
        }

        // メッシュのサイズを取得
        u64 GetMeshSize(const Mesh& m)
        {
            const u64 num_vertices{ m.vertices.size() };
            const u64 position_buffer_size{ m.position_buffer.size() };
            assert(position_buffer_size == sizeof(Math::v3) * num_vertices);
            const u64 element_buffer_size{ m.element_buffer.size() };
            assert(element_buffer_size == GetVertexElementSize(m.elements_type) * num_vertices);
            const u64 index_size{ (num_vertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };
            const u64 index_buffer_size{ index_size * m.indices.size() };
            constexpr u64 su32{ sizeof(u32) };
            const u64 size{
                su32 + m.name.size() +  // メッシュ名の長さとメッシュ名文字列のスペース
                su32 +                  // LOD id
                su32 +                  // 頂点要素サイズ（位置要素を除いた頂点サイズ）
                su32 +                  // 要素タイプ列挙
                su32 +                  // 頂点数
                su32 +                  // インデックスサイズ（16ビットまたは32ビット）
                su32 +                  // インデックス数
                sizeof(f32) +           // LOD しきい値
                position_buffer_size +  // 頂点位置の余裕
                element_buffer_size +   // 頂点要素の部屋
                index_buffer_size       // インデックスの余地
            };

            return size;
        }

        // シーンのサイズを取得
        u64 GetSceneSize(const Scene& scene)
        {
            constexpr u64 su32{ sizeof(u32) };
            u64 size
            {
                su32 +              // name length
                scene.name.size() + // room for scene name string
                su32                // number of LODs
            };

            for (const auto& lod : scene.lod_groups)
            {
                u64 lod_size
                {
                    su32 + lod.name.size() + // LOD name length and room for LPD name string
                    su32                     // number of meshes in this LOD
                };

                for (const auto& m : lod.meshes)
                {
                    lod_size += GetMeshSize(m);
                }

                size += lod_size;
            }

            return size;
        }

        // メッシュデータをバイナリにパック
        void PackMeshData(const Mesh& m, Utility::BlobStreamWriter& blob)
        {
            // mesh name
            blob.Write((u32)m.name.size());
            blob.Write(m.name.c_str(), m.name.size());
            // lod id
            blob.Write(m.lod_id);
            // vertex element size
            const u32 elements_size{ (u32)GetVertexElementSize(m.elements_type) };
            blob.Write(elements_size);
            // elements type enumeration
            blob.Write((u32)m.elements_type);
            // number of vertices
            const u32 num_vertices{ (u32)m.vertices.size() };
            blob.Write(num_vertices);
            // index size (16 bit or 32 bit)
            const u32 index_size{ (num_vertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };
            blob.Write(index_size);
            // number of indices
            const u32 num_indices{ (u32)m.indices.size() };
            blob.Write(num_indices);
            // LOD threshold
            blob.Write(m.lod_threshold);
            // position buffer
            assert(m.position_buffer.size() == sizeof(Math::v3) * num_vertices);
            blob.Write(m.position_buffer.data(), m.position_buffer.size());
            // element buffer
            assert(m.element_buffer.size() == elements_size * num_vertices);
            blob.Write(m.element_buffer.data(), m.element_buffer.size());
            // index data
            const u32 index_buffer_size{ index_size * num_indices };
            const u8* data{ (const u8*)m.indices.data() };
            Utility::vector<u16> indices;

            if (index_size == sizeof(u16)) {
                indices.resize(num_indices);
                for (u32 i{ 0 }; i < num_indices; ++i) indices[i] = (u16)m.indices[i];
                data = (const u8*)indices.data();
            }
            blob.Write(data, index_buffer_size);
        }

        // メッシュをマテリアルごとに分割
        bool SplitMeshesByMaterial(u32 material_idx, const Mesh& m, Mesh& submesh)
        {
            submesh.name = m.name;
            submesh.lod_threshold = m.lod_threshold;
            submesh.lod_id = m.lod_id;
            submesh.material_used.emplace_back(material_idx);
            submesh.uv_sets.resize(m.uv_sets.size());

            const u32 num_polys{ (u32)m.raw_indices.size() / 3 };
            Utility::vector<u32> vertex_ref(m.positions.size(), u32_invalid_id);

            for (u32 i{ 0 }; i < num_polys; ++i)
            {
                const u32 mtl_idx{ m.material_indices[i] };
                if (mtl_idx != material_idx) continue;

                const u32 index{ i * 3 };
                // 三角形を形成する各頂点をsubmeshに追加
                for (u32 j = index; j < index + 3; ++j)
                {
                    const u32 v_idx{ m.raw_indices[j] };
                    if (vertex_ref[v_idx] != u32_invalid_id) {
                        submesh.raw_indices.emplace_back(vertex_ref[v_idx]);
                    }
                    else {
                        submesh.raw_indices.emplace_back((u32)submesh.positions.size());
                        vertex_ref[v_idx] = submesh.raw_indices.back();
                        submesh.positions.emplace_back(m.positions[v_idx]);
                    }

                    if (m.normals.size()) {
                        submesh.normals.emplace_back(m.normals[j]);
                    }

                    if (m.tangents.size()) {
                        submesh.tangents.emplace_back(m.tangents[j]);
                    }

                    for (u32 k{ 0 }; k < m.uv_sets.size(); ++k)
                    {
                        if (m.uv_sets[k].size()) {
                            submesh.uv_sets[k].emplace_back(m.uv_sets[k][j]);
                        }
                    }
                }
            }

            assert((submesh.raw_indices.size() % 3) == 0);
            return !submesh.raw_indices.empty();
        }

        // Sceneに含まれるメッシュをマテリアルごとに分割
        void SplitMeshesByMaterial(Scene& scene, Progression* const progression)
        {
            assert(progression);
            progression->Callback(0, 0);

            for (auto& lod : scene.lod_groups)
            {
                Utility::vector<Mesh> new_meshes;

                for (const auto& m : lod.meshes)
                {
                    // このメッシュに複数のマテリアルが使用されている場合は、
                    // サブメッシュに分割する
                    const u32 num_materials{ (u32)m.material_used.size() };
                    if (num_materials > 1) {
                        for (u32 i{ 0 }; i < num_materials; ++i)
                        {
                            Mesh submesh{};
                            if (SplitMeshesByMaterial(m.material_used[i], m, submesh)) {
                                new_meshes.emplace_back(submesh);
                            }
                        }
                    }
                    else {
                        new_meshes.emplace_back(m);
                    }
                }

                progression->Callback(progression->value(), progression->max_value() + (u32)new_meshes.size());
                new_meshes.swap(lod.meshes);
            }
        }

        // dstの末尾にsrcを追加
        template <typename T> void AppendToVectorPod(Utility::vector<T>& dst, const Utility::vector<T>& src)
        {
            if (src.empty()) return;
            const u32 num_elements{ (u32)dst.size() };
            dst.resize(dst.size() + src.size());
            memcpy(&dst[num_elements], src.data(), src.size() * sizeof(T));
        }

    } // anonymous namespace

    void ProcessScene(Scene& scene, const GeometryImportSettings& settings, Progression* const progression)
    {
        assert(progression);
        SplitMeshesByMaterial(scene, progression);

        for (auto& lod : scene.lod_groups) {
            for (auto& m : lod.meshes)
            {
                ProcessVertices(m, settings);
                progression->Callback(progression->value() + 1, progression->max_value());
            }
        }
    }

    void PackData(const Scene& scene, SceneData& data)
    {
        const u64 scene_size{ GetSceneSize(scene) };
        data.buffer_size = (u32)scene_size;
        //IMalloc::Alloc と同じ方法でタスク メモリのブロックを割り当てます。
        data.buffer = (u8*)CoTaskMemAlloc(scene_size);
        assert(data.buffer);

        Utility::BlobStreamWriter blob{ data.buffer, data.buffer_size };

        // シーン名
        blob.Write((u32)scene.name.size());
        blob.Write(scene.name.c_str(), scene.name.size());
        // LOD数
        blob.Write((u32)scene.lod_groups.size());

        // このシーンに含まれるメッシュをバイナリにパック
        for (const auto& lod : scene.lod_groups)
        {
            // LOD名
            blob.Write((u32)lod.name.size());
            blob.Write(lod.name.c_str(), lod.name.size());
            // このLODのメッシュ数
            blob.Write((u32)lod.meshes.size());

            for (const auto& m : lod.meshes)
            {
                PackMeshData(m, blob);
            }
        }

        assert(scene_size == blob.Offset());
    }

    // メッシュの結合
    bool CoalesceMeshes(const LODGroup& lod, Mesh& combined_mesh, Progression* const progression)
    {
        assert(lod.meshes.size());
        const Mesh& first_mesh{ lod.meshes[0] };
        combined_mesh.name = first_mesh.name;
        combined_mesh.elements_type = DetermineElementsType(first_mesh);
        combined_mesh.lod_threshold = first_mesh.lod_threshold;
        combined_mesh.lod_id = first_mesh.lod_id;
        combined_mesh.uv_sets.resize(first_mesh.uv_sets.size());

        // メッシュの結合が可能かどうかを確認
        for (u32 mesh_idx{ 0 }; mesh_idx < lod.meshes.size(); ++mesh_idx)
        {
            const Mesh& m{ lod.meshes[mesh_idx] };

            if (combined_mesh.elements_type != DetermineElementsType(m) ||
                combined_mesh.uv_sets.size() != m.uv_sets.size() ||
                combined_mesh.lod_id != m.lod_id ||
                !Math::IsEqual(combined_mesh.lod_threshold, m.lod_threshold))
            {
                combined_mesh = {};
                return false;
            }
        }

        // メッシュの結合
        for (u32 mesh_idx{ 0 }; mesh_idx < lod.meshes.size(); ++mesh_idx)
        {
            const Mesh& m{ lod.meshes[mesh_idx] };

            const u32 position_count{ (u32)combined_mesh.positions.size() };
            const u32 raw_index_base{ (u32)combined_mesh.raw_indices.size() };

            AppendToVectorPod(combined_mesh.positions, m.positions);
            AppendToVectorPod(combined_mesh.normals, m.normals);
            AppendToVectorPod(combined_mesh.tangents, m.tangents);
            AppendToVectorPod(combined_mesh.colors, m.colors);

            for (u32 i{ 0 }; i < combined_mesh.uv_sets.size(); ++i)
            {
                AppendToVectorPod(combined_mesh.uv_sets[i], m.uv_sets[i]);
            }

            AppendToVectorPod(combined_mesh.material_indices, m.material_indices);
            AppendToVectorPod(combined_mesh.raw_indices, m.raw_indices);

            for (u32 i{ raw_index_base }; i < combined_mesh.raw_indices.size(); ++i)
            {
                combined_mesh.raw_indices[i] += position_count;
            }

            progression->Callback(progression->value(), progression->max_value() > 1 ? progression->max_value() - 1 : 1);
        }

        // マテリアルのインデックスを登録
        for (const u32 mtl_idx : combined_mesh.material_indices)
        {
            if (std::find(combined_mesh.material_used.begin(), combined_mesh.material_used.end(), mtl_idx) == combined_mesh.material_used.end()) {
                combined_mesh.material_used.emplace_back(mtl_idx);
            }
        }

        return true;
    }

} // namespace Content