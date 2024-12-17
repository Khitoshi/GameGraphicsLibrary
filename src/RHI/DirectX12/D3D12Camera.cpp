#include "D3D12Camera.h"
#include "../EngineAPI/GameEntity.h"

namespace RHI::D3D12::Camera {
    namespace {

        Utility::free_list<D3D12Camera> cameras;
#pragma region Setter
        void SetUpVector(D3D12Camera camera, const void* const data, const u32 size)
        {
            Math::v3 up_vector{ *(Math::v3*)data };
            assert(sizeof(up_vector) == size);
            camera.Up(up_vector);
        }

        void SetFieldOfView(D3D12Camera camera, const void* const data, const u32 size)
        {
            assert(camera.ProjectionType() == Graphics::Camera::PERSPECTIVE);
            f32 fov{ *(f32*)data };
            assert(sizeof(fov) == size);
            camera.FieldOfView(fov);
        }

        void SetAspectRatio(D3D12Camera camera, const void* const data, const u32 size)
        {
            assert(camera.ProjectionType() == Graphics::Camera::PERSPECTIVE);
            f32 aspect_ratio{ *(f32*)data };
            assert(sizeof(aspect_ratio) == size);
            camera.AspectRatio(aspect_ratio);
        }

        void SetViewWidth(D3D12Camera camera, const void* const data, const u32 size)
        {
            assert(camera.ProjectionType() == Graphics::Camera::ORTHOGRAPHIC);
            f32 view_width{ *(f32*)data };
            assert(sizeof(view_width) == size);
            camera.ViewWidth(view_width);
        }

        void SetViewHeight(D3D12Camera camera, const void* const data, const u32 size)
        {
            assert(camera.ProjectionType() == Graphics::Camera::ORTHOGRAPHIC);
            f32 view_height{ *(f32*)data };
            assert(sizeof(view_height) == size);
            camera.ViewHeight(view_height);
        }

        void SetNearZ(D3D12Camera camera, const void* const data, const u32 size)
        {
            f32 near_z{ *(f32*)data };
            assert(sizeof(near_z) == size);
            camera.NearZ(near_z);
        }

        void SetFarZ(D3D12Camera camera, const void* const data, const u32 size)
        {
            f32 far_z{ *(f32*)data };
            assert(sizeof(far_z) == size);
            camera.FarZ(far_z);
        }
#pragma endregion// Setter
#pragma region Getter
        void GetView(D3D12Camera camera, void* const data, const u32 size)
        {
            Math::m4x4* const matrix{ (Math::m4x4* const)data };
            assert(sizeof(Math::m4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.View());
        }

        void GetProjection(D3D12Camera camera, void* const data, const u32 size)
        {
            Math::m4x4* const matrix{ (Math::m4x4* const)data };
            assert(sizeof(Math::m4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.Projection());
        }

        void GetInverseProjection(D3D12Camera camera, void* const data, const u32 size)
        {
            Math::m4x4* const matrix{ (Math::m4x4* const)data };
            assert(sizeof(Math::m4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.InverseProjection());
        }

        void GetViewProjection(D3D12Camera camera, void* const data, const u32 size)
        {
            Math::m4x4* const matrix{ (Math::m4x4* const)data };
            assert(sizeof(Math::m4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.ViewProjection());
        }

        void GetInverseViewProjection(D3D12Camera camera, void* const data, const u32 size)
        {
            Math::m4x4* const matrix{ (Math::m4x4* const)data };
            assert(sizeof(Math::m4x4) == size);
            DirectX::XMStoreFloat4x4(matrix, camera.InverseViewProjection());
        }

        void GetUpVector(D3D12Camera camera, void* const data, const u32 size)
        {
            Math::v3* const up_vector{ (Math::v3* const)data };
            assert(sizeof(Math::v3) == size);
            DirectX::XMStoreFloat3(up_vector, camera.Up());
        }

        void GetFieldOfView(D3D12Camera camera, void* const data, const u32 size)
        {
            assert(camera.ProjectionType() == Graphics::Camera::PERSPECTIVE);
            f32* const fov{ (f32* const)data };
            assert(sizeof(f32) == size);
            *fov = camera.FieldOfView();
        }

        void GetAspectRatio(D3D12Camera camera, void* const data, const u32 size)
        {
            assert(camera.ProjectionType() == Graphics::Camera::PERSPECTIVE);
            f32* const aspect_ratio{ (f32* const)data };
            assert(sizeof(f32) == size);
            *aspect_ratio = camera.AspectRatio();
        }

        void GetViewWidth(D3D12Camera camera, void* const data, const u32 size)
        {
            assert(camera.ProjectionType() == Graphics::Camera::ORTHOGRAPHIC);
            f32* const view_width{ (f32* const)data };
            assert(sizeof(f32) == size);
            *view_width = camera.ViewWidth();
        }

        void GetViewHeight(D3D12Camera camera, void* const data, const u32 size)
        {
            assert(camera.ProjectionType() == Graphics::Camera::ORTHOGRAPHIC);
            f32* const view_height{ (f32* const)data };
            assert(sizeof(f32) == size);
            *view_height = camera.ViewHeight();
        }

        void GetNearZ(D3D12Camera Camera, void* const data, const u32 size)
        {
            f32* const near_z{ (f32* const)data };
            assert(sizeof(f32) == size);
            *near_z = Camera.NearZ();
        }

        void GetFarZ(D3D12Camera camera, void* const data, const u32 size)
        {
            f32* const far_z{ (f32* const)data };
            assert(sizeof(f32) == size);
            *far_z = camera.FarZ();
        }

        void GetProjectionType(D3D12Camera camera, void* const data, const u32 size)
        {
            Graphics::Camera::Type* const type{ (Graphics::Camera::Type* const)data };
            assert(sizeof(Graphics::Camera::Type) == size);
            *type = camera.ProjectionType();
        }

        void GetEntityID(D3D12Camera camera, void* const data, const u32 size)
        {
            Id::id_type* const entity_id{ (Id::id_type* const)data };
            assert(sizeof(Id::id_type) == size);
            *entity_id = camera.EntityID();
        }
#pragma endregion// Getter
        void DummySet(D3D12Camera, const void* const, const u32)
        {}

        using SetFunction = void(*)(D3D12Camera, const void* const, u32);
        using GetFunction = void(*)(D3D12Camera, void* const, const u32);
        constexpr SetFunction set_functions[]
        {
            SetUpVector,
            SetFieldOfView,
            SetAspectRatio,
            SetViewWidth,
            SetViewHeight,
            SetNearZ,
            SetFarZ,
            DummySet,
            DummySet,
            DummySet,
            DummySet,
            DummySet,
            DummySet,
            DummySet,
        };

        static_assert(_countof(set_functions) == CameraParameter::COUNT);

        constexpr GetFunction get_functions[]
        {
            GetUpVector,
            GetFieldOfView,
            GetAspectRatio,
            GetViewWidth,
            GetViewHeight,
            GetNearZ,
            GetFarZ,
            GetView,
            GetProjection,
            GetInverseProjection,
            GetViewProjection,
            GetInverseViewProjection,
            GetProjectionType,
            GetEntityID,
        };

        static_assert(_countof(get_functions) == CameraParameter::COUNT);
    }


    D3D12Camera::D3D12Camera(const CameraInitInfo info) :
        up_{ DirectX::XMLoadFloat3(&info.up) },
        near_z_{ info.near_z },
        far_z_{ info.far_z },
        field_of_view_{ info.field_of_view },
        aspect_ratio_{ info.aspect_ratio },
        projection_type_{ info.type },
        entity_id_{ info.entity_id },
        is_dirty_{ true }
    {
        assert(Id::IsValid(entity_id_));
        Update();
    }

    void D3D12Camera::Update()
    {
        GameEntity::Entity entity{ GameEntity::EntityID{entity_id_} };
        using namespace DirectX;
        Math::v3 pos{ entity.Transform().Position() };
        Math::v3 dir{ entity.Transform().Orientation() };
        XMVECTOR position{ XMLoadFloat3(&pos) };
        XMVECTOR direction{ XMLoadFloat3(&dir) };
        view_ = XMMatrixLookAtRH(position, direction, up_);

        if (is_dirty_)
        {
            projection_ = (projection_type_ == Graphics::Camera::PERSPECTIVE)
                ? XMMatrixPerspectiveFovRH(field_of_view_ * XM_PI, aspect_ratio_, near_z_, far_z_)
                : XMMatrixPerspectiveRH(view_width_, view_height_, near_z_, far_z_);
            inverse_projection_ = XMMatrixInverse(nullptr, projection_);
            is_dirty_ = false;
        }

        view_projection_ = XMMatrixMultiply(view_, projection_);
        inverse_view_projection_ = XMMatrixInverse(nullptr, view_projection_);
    }

    void D3D12Camera::Up(Math::v3 up)
    {
        up_ = DirectX::XMLoadFloat3(&up);
    }

    void D3D12Camera::FieldOfView(f32 fov)
    {
        assert(projection_type_ == Graphics::Camera::PERSPECTIVE);
        field_of_view_ = fov;
        is_dirty_ = true;
    }

    void D3D12Camera::AspectRatio(f32 aspect_ratio)
    {
        assert(projection_type_ == Graphics::Camera::PERSPECTIVE);
        aspect_ratio_ = aspect_ratio;
        is_dirty_ = true;
    }

    void D3D12Camera::ViewWidth(f32 width)
    {
        assert(width);
        assert(projection_type_ == Graphics::Camera::ORTHOGRAPHIC);
        view_width_ = width;
        is_dirty_ = true;
    }

    void D3D12Camera::ViewHeight(f32 height)
    {
        assert(height);
        assert(projection_type_ == Graphics::Camera::ORTHOGRAPHIC);
        view_height_ = height;
        is_dirty_ = true;
    }

    void D3D12Camera::NearZ(f32 near_z)
    {
        near_z_ = near_z;
        is_dirty_ = true;
    }

    void D3D12Camera::FarZ(f32 far_z)
    {
        far_z_ = far_z;
        is_dirty_ = true;
    }

    Graphics::Camera Create(CameraInitInfo info)
    {
        return Graphics::Camera{ Graphics::CameraID{ cameras.add(info) } };
    }

    void Remove(Graphics::CameraID id)
    {
        assert(Id::IsValid(id));
        cameras.remove(id);
    }

    void SetParameter(Graphics::CameraID id, CameraParameter::PARAMETER parameter, const void* const data, u32 data_size)
    {
        assert(data && data_size);
        assert(parameter < CameraParameter::COUNT);
        D3D12Camera& camera{ Get(id) };
        set_functions[parameter](camera, data, data_size);
    }

    void GetParameter(Graphics::CameraID id, CameraParameter::PARAMETER parameter, void* const data, u32 data_size)
    {
        assert(data && data_size);
        assert(parameter < CameraParameter::COUNT);
        D3D12Camera& camera{ Get(id) };
        get_functions[parameter](camera, data, data_size);
    }

    D3D12Camera& Get(Graphics::CameraID id)
    {
        assert(Id::IsValid(id));
        return cameras[id];
    }

}

