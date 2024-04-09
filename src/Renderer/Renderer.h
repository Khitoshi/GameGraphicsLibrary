#pragma once

#include "CommonHeaders.h"
#include "..\Platform\Window.h"

namespace Renderer {

    DEFINE_TYPED_ID(SurfaceID);
    class Surface
    {
    public:
        constexpr explicit Surface(SurfaceID id) : id_{ id } {}
        constexpr Surface() = default;
        constexpr SurfaceID get_id() const { return id_; }
        constexpr bool is_valid() const { return Id::IsValid(id_); }

        void Resize(u32 width, u32 height) const;
        u32 Width() const;
        u32 Height() const;
        void Render() const;
    private:
        SurfaceID id_{ Id::invalid_id };
    };

    struct RenderSurface
    {
        Platform::Window window{};
        Surface surface{};
    };

    enum class GRAPHICS_PLATFORM :u32
    {
        direct3d12 = 0,
    };

    bool Initialize(GRAPHICS_PLATFORM platform);
    void Shutdown();

    Surface CreateSurface(Platform::Window window);
    void RemoveSurface(SurfaceID id);
}