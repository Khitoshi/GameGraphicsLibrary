#include "D3D12Interface.h"
#include "../RHIPlatformInterface.h"
#include "D3D12Core.h"
namespace RHI::D3D12
{
    void GetPlatformInterface(RHIPlatformInterface& rhi_platform_interface)
    {
        rhi_platform_interface.Initialize = Core::Initialize;
        rhi_platform_interface.Shutdown = Core::Shutdown;


        //rhi_platform_interface.Render = Core::Render;

        rhi_platform_interface.surface.Create = Core::CreateSurface;
        rhi_platform_interface.surface.Remove = Core::RemoveSurface;
        rhi_platform_interface.surface.Resize = Core::ResizeSurface;
        rhi_platform_interface.surface.Width = Core::SurfaceWidth;
        rhi_platform_interface.surface.Height = Core::SurfaceHeight;
        rhi_platform_interface.surface.Render = Core::RenderSurface;

    }

}