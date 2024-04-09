#include "Renderer.h"
#include "../RHI/RHIPlatformInterface.h"
#include "../RHI/DirectX12/D3D12Interface.h"

namespace Renderer
{
    namespace {
        //RHI::d3d12::PlatformInterface gfx{};
        RHI::RHIPlatformInterface gfx{};

        bool SetPlatformInterface(GRAPHICS_PLATFORM platform)
        {
            switch (platform)
            {
            case GRAPHICS_PLATFORM::direct3d12:
                RHI::D3D12::GetPlatformInterface(gfx);
                break;
            default:
                return false;
            }

            return true;
        }
    }

    bool Initialize(GRAPHICS_PLATFORM platform)
    {
        return SetPlatformInterface(platform) && gfx.Initialize();
    }

    void Shutdown()
    {
        gfx.Shutdown();
    }

    Surface CreateSurface(Platform::Window window)
    {
        return gfx.surface.Create(window);
    }

    void RemoveSurface(SurfaceID id)
    {
        assert(Id::IsValid(id));
        gfx.surface.Remove(id);
    }

    void Surface::Resize(u32 width, u32 height) const
    {
        assert(is_valid());
        gfx.surface.Resize(id_, width, height);
    }

    u32 Surface::Width() const
    {
        assert(is_valid());
        return gfx.surface.Width(id_);
    }

    u32 Surface::Height() const
    {
        assert(is_valid());
        return gfx.surface.Height(id_);
    }

    void Surface::Render() const
    {
        assert(is_valid());
        return gfx.surface.Render(id_);
    }


}

