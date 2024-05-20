#pragma once
#include "D3D12CommonHeaders.h"
#include "../EngineAPI/Camera.h"

namespace RHI::D3D12::Camera {

    struct CameraParameter {
        enum PARAMETER : u32 {
            UP_VECTOR,
            FIELD_OF_VIEW,
            ASPECT_RATIO,
            VIEW_WIDTH,
            VIEW_HEIGHT,
            NEAR_Z,
            FAR_Z,
            VIEW,
            PROJECTION,
            INVERSE_PROJECTION,
            VIEW_PROJECTION,
            INVERSE_VIEW_PROJECTION,
            TYPE,
            ENTITY_ID,

            COUNT
        };
    };

    struct CameraInitInfo
    {
        //camera_init_info() = delete;
        Id::id_type entity_id{ Id::invalid_id };
        Graphics::Camera::Type type{};
        Math::v3 up;
        union {
            f32 field_of_view;
            f32 view_width;
        };
        union {
            f32 aspect_ratio;
            f32 view_height;
        };
        f32 near_z;
        f32 far_z;
    };

    struct PerspectiveCameraInitInfo : public CameraInitInfo
    {
        explicit PerspectiveCameraInitInfo(Id::id_type id)
        {
            assert(Id::IsValid(id));
            entity_id = id;
            type = Graphics::Camera::PERSPECTIVE;
            up = { 0.f, 1.f, 0.f };
            field_of_view = 0.25f;
            aspect_ratio = 16.f / 10.f;
            near_z = 0.001f;
            far_z = 10000.f;
        }
    };

    struct OrthographicCameraInitInfo : public CameraInitInfo
    {
        explicit OrthographicCameraInitInfo(Id::id_type id)
        {
            assert(Id::IsValid(id));
            entity_id = id;
            type = Graphics::Camera::PERSPECTIVE;
            up = { 0.f, 1.f, 0.f };
            view_width = 1920;
            view_height = 1080;
            near_z = 0.001f;
            far_z = 10000.f;
        }
    };

    class D3D12Camera
    {
    public:
        explicit D3D12Camera(CameraInitInfo info);

        void Update();
        void Up(Math::v3 up);
        void FieldOfView(f32 fov);
        void AspectRatio(f32 aspect_ratio);
        void ViewWidth(f32 width);
        void ViewHeight(f32 height);
        void NearZ(f32 near_z);
        void FarZ(f32 far_z);

#pragma region Getter
        [[nodiscard]] constexpr DirectX::XMMATRIX       View()                  const { return view_; }
        [[nodiscard]] constexpr DirectX::XMMATRIX       Projection()            const { return projection_; }
        [[nodiscard]] constexpr DirectX::XMMATRIX       InverseProjection()     const { return inverse_projection_; }
        [[nodiscard]] constexpr DirectX::XMMATRIX       ViewProjection()        const { return view_projection_; }
        [[nodiscard]] constexpr DirectX::XMMATRIX       InverseViewProjection() const { return inverse_view_projection_; }
        [[nodiscard]] constexpr DirectX::XMVECTOR       Up()                    const { return up_; }
        [[nodiscard]] constexpr f32                     NearZ()                 const { return near_z_; }
        [[nodiscard]] constexpr f32                     FarZ()                  const { return far_z_; }
        [[nodiscard]] constexpr f32                     FieldOfView()           const { return field_of_view_; }
        [[nodiscard]] constexpr f32                     AspectRatio()           const { return aspect_ratio_; }
        [[nodiscard]] constexpr f32                     ViewWidth()             const { return view_width_; }
        [[nodiscard]] constexpr f32                     ViewHeight()            const { return view_height_; }
        [[nodiscard]] constexpr Graphics::Camera::Type  ProjectionType()        const { return projection_type_; }
        [[nodiscard]] constexpr Id::id_type             EntityID()              const { return entity_id_; }
#pragma endregion// Getter
    private:
        DirectX::XMMATRIX       view_;
        DirectX::XMMATRIX       projection_;
        DirectX::XMMATRIX       inverse_projection_;
        DirectX::XMMATRIX       view_projection_;
        DirectX::XMMATRIX       inverse_view_projection_;
        DirectX::XMVECTOR       up_;
        f32                     near_z_;
        f32                     far_z_;
        union {
            f32                 field_of_view_;     // 遠近カメラの視野
            f32                 view_width_;        // 正射影カメラの視野幅（ピクセル
        };
        union {
            f32                 aspect_ratio_;      // 遠近カメラの縦横比
            f32                 view_height_;       // 正射投影カメラのビューの高さ（ピクセル
        };
        Graphics::Camera::Type  projection_type_;
        Id::id_type             entity_id_;
        bool                    is_dirty_;
    };

    Graphics::Camera Create(CameraInitInfo info);
    void Remove(Graphics::CameraID id);
    void SetParameter(Graphics::CameraID id, CameraParameter::PARAMETER parameter, const void* const data, u32 data_size);
    void GetParameter(Graphics::CameraID id, CameraParameter::PARAMETER parameter, void* const data, u32 data_size);
    [[nodiscard]] D3D12Camera& Get(Graphics::CameraID id);

};
