#include "Camera.h"
#include "../Renderer/Renderer.h"
#include "../RHI/RHIPlatformInterface.h"

namespace Graphics
{


    void Camera::Up(Math::v3 up) const
    {
        assert(IsValid());
        Renderer::GetPlatformInterface()->camera.SetParameter(id_, RHI::D3D12::Camera::CameraParameter::UP_VECTOR, &up, sizeof(up));
    }

    void Camera::FieldOfView(f32 fov) const
    {
        assert(IsValid());
        Renderer::GetPlatformInterface()->camera.SetParameter(id_, RHI::D3D12::Camera::CameraParameter::FIELD_OF_VIEW, &fov, sizeof(fov));
    }

    void Camera::AspectRatio(f32 aspect_ratio) const
    {
        assert(IsValid());
        Renderer::GetPlatformInterface()->camera.SetParameter(id_, RHI::D3D12::Camera::CameraParameter::ASPECT_RATIO, &aspect_ratio, sizeof(aspect_ratio));
    }

    void Camera::ViewWidth(f32 width) const
    {
        assert(IsValid());
        Renderer::GetPlatformInterface()->camera.SetParameter(id_, RHI::D3D12::Camera::CameraParameter::VIEW_WIDTH, &width, sizeof(width));
    }

    void Camera::ViewHeight(f32 height) const
    {
        assert(IsValid());
        Renderer::GetPlatformInterface()->camera.SetParameter(id_, RHI::D3D12::Camera::CameraParameter::VIEW_HEIGHT, &height, sizeof(height));
    }

    void Camera::Range(f32 near_z, f32 far_z) const
    {
        assert(IsValid());
        Renderer::GetPlatformInterface()->camera.SetParameter(id_, RHI::D3D12::Camera::CameraParameter::NEAR_Z, &near_z, sizeof(near_z));
        Renderer::GetPlatformInterface()->camera.SetParameter(id_, RHI::D3D12::Camera::CameraParameter::FAR_Z, &far_z, sizeof(far_z));
    }

    Math::m4x4 Camera::View() const
    {
        assert(IsValid());
        Math::m4x4 matrix;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::VIEW, &matrix, sizeof(matrix));
        return matrix;
    }

    Math::m4x4 Camera::Projection() const
    {
        assert(IsValid());
        Math::m4x4 matrix;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::PROJECTION, &matrix, sizeof(matrix));
        return matrix;
    }

    Math::m4x4 Camera::InverseProjection() const
    {
        assert(IsValid());
        Math::m4x4 matrix;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::INVERSE_PROJECTION, &matrix, sizeof(matrix));
        return matrix;
    }

    Math::m4x4 Camera::ViewProjection() const
    {
        assert(IsValid());
        Math::m4x4 matrix;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::VIEW_PROJECTION, &matrix, sizeof(matrix));
        return matrix;
    }

    Math::m4x4 Camera::InverseViewProjection() const
    {
        assert(IsValid());
        Math::m4x4 matrix;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::INVERSE_VIEW_PROJECTION, &matrix, sizeof(matrix));
        return matrix;
    }

    Math::v3 Camera::Up() const
    {
        assert(IsValid());
        Math::v3 up_vector;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::UP_VECTOR, &up_vector, sizeof(up_vector));
        return up_vector;
    }

    f32 Camera::NearZ() const
    {
        assert(IsValid());
        f32 near_z;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::NEAR_Z, &near_z, sizeof(near_z));
        return near_z;
    }

    f32 Camera::FarZ() const
    {
        assert(IsValid());
        f32 far_z;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::FAR_Z, &far_z, sizeof(far_z));
        return far_z;
    }

    f32 Camera::FieldOfView() const
    {
        assert(IsValid());
        f32 fov;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::FIELD_OF_VIEW, &fov, sizeof(fov));
        return fov;
    }

    f32 Camera::AspectRatio() const
    {
        assert(IsValid());
        f32 aspect_ratio;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::ASPECT_RATIO, &aspect_ratio, sizeof(aspect_ratio));
        return aspect_ratio;
    }

    f32 Camera::ViewWidth() const
    {
        assert(IsValid());
        f32 width;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::VIEW_WIDTH, &width, sizeof(width));
        return width;
    }

    f32 Camera::ViewHeight() const
    {
        assert(IsValid());
        f32 height;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::VIEW_HEIGHT, &height, sizeof(height));
        return height;
    }

    Camera::Type Camera::ProjectionType() const
    {
        assert(IsValid());
        Camera::Type type;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::TYPE, &type, sizeof(type));
        return type;
    }

    Id::id_type Camera::EntityID() const
    {
        assert(IsValid());
        Id::id_type entity_id;
        Renderer::GetPlatformInterface()->camera.GetParameter(id_, RHI::D3D12::Camera::CameraParameter::ENTITY_ID, &entity_id, sizeof(entity_id));
        return entity_id;
    }

}
