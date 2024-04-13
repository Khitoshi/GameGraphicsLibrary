#pragma once

#include "D3D12CommonHeaders.h"

// ëOï˚êÈåæ
namespace RHI::D3D12::Parts {
    class DescriptorHeapManager;
    class D3D12DescriptorHeap;
}

namespace Platform {
    class Window;
}

namespace Renderer {
    class Surface;
    struct SurfaceID;
}


namespace RHI::D3D12::Core {
    namespace Detail {
        void DeferredRelease(IUnknown* resource);
    }

    bool Initialize();
    void Shutdown();

    Renderer::Surface CreateSurface(Platform::Window window);
    void RemoveSurface(Renderer::SurfaceID id);
    void ResizeSurface(Renderer::SurfaceID id, u32, u32);
    u32 SurfaceWidth(Renderer::SurfaceID id);
    u32 SurfaceHeight(Renderer::SurfaceID id);
    void RenderSurface(Renderer::SurfaceID id);

#pragma region Getter
    ID3D12Device* const GetDevice();
    u32 GetCurrentFrameIndex();

    RHI::D3D12::Parts::DescriptorHeapManager* GetDescriptorHeapManager();
    RHI::D3D12::Parts::D3D12DescriptorHeap& GetSRVHeap();
    RHI::D3D12::Parts::D3D12DescriptorHeap& GetRTVHeap();
    RHI::D3D12::Parts::D3D12DescriptorHeap& GetDSVHeap();
    DXGI_FORMAT GetDefaultRenderTargetFormat();
#pragma endregion//Getter

#pragma region Setter
    void SetDeferredReleasesFlag();
#pragma endregion //Setter

}