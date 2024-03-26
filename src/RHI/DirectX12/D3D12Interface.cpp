#include "D3D12Interface.h"
#include "../RHIPlatformInterface.h"
#include "D3D12Core.h"
namespace RHI::D3D12
{
    void GetPlatformInterface(RHIPlatformInterface& rhi_platform_interface)
    {
        rhi_platform_interface.Initialize = Core::Initialize;
        rhi_platform_interface.Shutdown = Core::Shutdown;
        rhi_platform_interface.Render = Core::Render;
    }

}