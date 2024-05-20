#pragma once

#include "CommonHeaders.h"

namespace Graphics
{
    DEFINE_TYPED_ID(CameraID);

    class Camera
    {
    public:
        enum Type : u32
        {
            PERSPECTIVE,    // ìßéãìäâe
            ORTHOGRAPHIC    // ê≥éÀâe
        };

        constexpr explicit Camera(CameraID id) : id_{ id } {}
        constexpr Camera() = default;
        constexpr CameraID GetID() const { return id_; }
        constexpr bool      IsValid() const { return Id::IsValid(id_); }

        void        Up(Math::v3 up) const;
        void        FieldOfView(f32 fov) const;
        void        AspectRatio(f32 aspect_ratio) const;
        void        ViewWidth(f32 width) const;
        void        ViewHeight(f32 height) const;
        void        Range(f32 near_z, f32 far_z) const;

        Math::m4x4  View() const;
        Math::m4x4  Projection() const;
        Math::m4x4  InverseProjection() const;
        Math::m4x4  ViewProjection() const;
        Math::m4x4  InverseViewProjection() const;
        Math::v3    Up() const;
        f32         NearZ() const;
        f32         FarZ() const;
        f32         FieldOfView() const;
        f32         AspectRatio() const;
        f32         ViewWidth() const;
        f32         ViewHeight() const;
        Type        ProjectionType() const;
        Id::id_type EntityID() const;

    private:
        CameraID id_{ Id::invalid_id };
    };
}
