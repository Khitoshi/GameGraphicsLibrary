#pragma once
#include "CommonHeaders.h"
#include "D3D12Helpers.h"

namespace RHI::D3D12::Parts {
    struct FrameInfo;
    class D3D12RenderTexture;
    class D3D12DepthBuffer;
}

namespace RHI::D3D12::GPASS {

    bool Initialize();
    void Shutdown();

    void SetSize(Math::u32v2 size);
    void DepthPrepass(ID3D12GraphicsCommandList6* cmd_list, const Parts::FrameInfo& info);
    void Render(ID3D12GraphicsCommandList6* cmd_list, const Parts::FrameInfo& info);

    void AddTransitionsForDepthPrepass(D3DX::ResourceBarrier& barriers);
    void AddTransitionsForGpass(D3DX::ResourceBarrier& barriers);
    void AddTransitionsForPostProcess(D3DX::ResourceBarrier& barriers);

    void SetRenderTargetsForDepthPrepass(ID3D12GraphicsCommandList6* cmd_list);
    void SetRenderTargetsForGpass(ID3D12GraphicsCommandList6* cmd_list);

#pragma region Getter
    [[nodiscard]] const Parts::D3D12RenderTexture& MainBuffer();
    [[nodiscard]] const Parts::D3D12DepthBuffer& DepthBuffer();
#pragma endregion //Getter


}