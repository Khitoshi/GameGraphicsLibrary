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
}

